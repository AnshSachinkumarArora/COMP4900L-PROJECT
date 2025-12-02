#include "ofApp.h"

void ofApp::setup() {
	ofSetBackgroundColor(ofColor::black);
	easyCam.setDistance(800);

	// --- Terrain Setup (Same as before) ---
	int res = 80;
	terrain.setMode(OF_PRIMITIVE_TRIANGLES);
	for (int iy = 0; iy <= res; ++iy) {
		for (int ix = 0; ix <= res; ++ix) {
			float x = ix * terrain_size / res;
			float y = iy * terrain_size / res;
			float z = getTerrainHeight(x, y);
			terrain.addVertex(ofVec3f(x, y, z));

			float shore_t = (y - shoreline_y) / terrain_size;
			if (shore_t > 0)
				terrain.addColor(ofColor::fromHex(0x2f2f2f));
			else
				terrain.addColor(ofColor::fromHex(0x5c5c5c));
		}
	}
	for (int iy = 0; iy < res; ++iy) {
		for (int ix = 0; ix < res; ++ix) {
			int i = iy * (res + 1) + ix;
			terrain.addTriangle(i, i + 1, i + res + 1);
			terrain.addTriangle(i + res + 1, i + 1, i + res + 2);
		}
	}

	// Generate initial distribution
	generateDistribution();
}

void ofApp::update() {
}

void ofApp::draw() {
	easyCam.begin();
	ofPushMatrix();
	ofTranslate(-terrain_size / 2, -terrain_size / 2); // Center terrain

	terrain.draw();

	for (auto & d : debrisList) {
		ofSetColor(d.color);
		ofPushMatrix();
		// 1. Go to position
		ofTranslate(d.pos);
		// 2. TILT: Align the object's Up vector (Z) to the Terrain's Normal
		//calculate the axis of rotation and the angle needed
		ofVec3f axis = ofVec3f(0, 0, 1).crossed(d.slopeNormal);
		float angle = ofVec3f(0, 0, 1).angle(d.slopeNormal);
		ofRotateDeg(angle, axis.x, axis.y, axis.z);
		// 3. SPIN: Rotate around its own local center (The randomness)
		ofRotateZDeg(d.angle);
		// 4. Draw
		ofDrawBox(0, 0, 0, d.size.x, d.size.y, d.size.z);
		ofPopMatrix();
	}
	ofPopMatrix();
	easyCam.end();

	// On-screen instructions 
	ofSetColor(255);
	string msg = "Current Mode: ";
	if (currentMode == 1)
		msg += "1. Just Power Law (Scattered)";
	else if (currentMode == 2)
		msg += "2. Just Matern (Uniform Clusters)";
	else if (currentMode == 3)
		msg += "3. Matern + Power Law (Realistic)";
	msg += "\nPress 1, 2, 3 to switch. Space to Regenerate.";

	//disable depth test briefly to draw text on top
	ofDisableDepthTest();
	ofDrawBitmapString(msg, 10, 20);
	ofEnableDepthTest();
}

void ofApp::keyPressed(int key) {
	if (key == '1') {
		currentMode = 1;
		generateDistribution();
	}
	if (key == '2') {
		currentMode = 2;
		generateDistribution();
	}
	if (key == '3') {
		currentMode = 3;
		generateDistribution();
	}
	if (key == ' ') {
		generateDistribution(); // Regenerate current mode
	}
}

void ofApp::generateDistribution() {
	if (currentMode == 1)
		generateJustPowerLaw();
	else if (currentMode == 2)
		generateJustMatern();
	else if (currentMode == 3)
		generateMaternPowerLaw();
}

float ofApp::getTerrainHeight(float x, float y) {
	float slope = (y - shoreline_y) / terrain_size * 30.0f;
	float noise = ofNoise(x * 0.01f, y * 0.01f) * 20.0f;
	return slope + noise;
}

// ============================================================
// MODE 1: JUST POWER LAW
// Logic: Random placement (No clusters), but Size = Power Law
// ============================================================
void ofApp::generateJustPowerLaw() {
	debrisList.clear();
	int numObjects = 20000; // Lots of scattered objects

	for (int i = 0; i < numObjects; i++) {
		float x = ofRandom(terrain_size);
		float y = ofRandom(terrain_size);

		// Shoreline Gradient (still applies to everything)
		float distFromShore = abs(y - shoreline_y);
		float prob = ofMap(distFromShore, 0, terrain_size * 0.9, 1.0, 0.2, true);

		if (ofRandom(1.0f) < prob) {
			if (x < 5.0 || x > terrain_size - 15.0 || y < 5.0 || y > terrain_size * 0.91) continue;
			DebrisObject trash;
			float z = getTerrainHeight(x, y) + 1;
			trash.pos = ofVec3f(x, y, z);
			trash.slopeNormal = getTerrainNormal(x, y);

			// --- POWER LAW SIZE ---
			// Most objects are tiny, few are big.
			float k = 5.0f;
			float scale = pow(ofRandom(1.0f), k);
			float len = ofLerp(2.0f, 25.0f, scale);
			trash.size = ofVec3f(2, len, 2);

			trash.angle = ofRandom(360.0f);

			// Color based on size
			if (scale < 0.1)
				trash.color = ofColor::lightGray;
			else
				trash.color = ofColor::tan;

			debrisList.push_back(trash);
		}
	}
}

// ============================================================
// MODE 2: JUST MATERN
// Logic: Clustered placement, but Clusters are UNIFORM size
// ============================================================
void ofApp::generateJustMatern() {
	debrisList.clear();

	// Define centers
	int numCenters = 300;

	for (int c = 0; c < numCenters; c++) {
		// Pick a center (Weighted by shore)
		float cx = ofRandom(terrain_size);
		float cy = ofRandom(terrain_size);
		float dist = abs(cy - shoreline_y);
		if (ofRandom(1.0f) > ofMap(dist, 0, terrain_size * 0.9, 1.0, 0.2, true)) continue;

		// --- UNIFORM CLUSTER SIZE ---
		// Every pile looks roughly the same
		int numTrash = 40;
		float radius = 50.0f;

		for (int i = 0; i < numTrash; i++) {
			float r = sqrt(ofRandom(1.0f)) * radius;
			float theta = ofRandom(TWO_PI);
			float x = cx + r * cos(theta);
			float y = cy + r * sin(theta);

			if (x < 5.0 || x > terrain_size - 15.0 || y < 5.0 || y > terrain_size * 0.91) continue;

			DebrisObject trash;
			float z = getTerrainHeight(x, y) + 1;
			trash.pos = ofVec3f(x, y, z);
			trash.slopeNormal = getTerrainNormal(x, y);

			// Random sizes (No Power Law)
			trash.size = ofVec3f(2, ofRandom(5, 25), 2);
			trash.angle = ofRandom(360.0f);
			if (ofRandom(1.0) <= 0.5)
				trash.color = ofColor::gray;
			else
				trash.color = ofColor::tan;

			debrisList.push_back(trash);
		}
	}
}

// ============================================================
// MODE 3: MATERN + POWER LAW (The Pro Version)
// Logic: Clusters + Power Law determining Cluster Intensity
// ============================================================
void ofApp::generateMaternPowerLaw() {
	debrisList.clear();

	// Define centers
	int numCenters = 400;

	for (int c = 0; c < numCenters; c++) {
		float cx = ofRandom(terrain_size);
		float cy = ofRandom(terrain_size);
		float dist = abs(cy - shoreline_y);
		if (ofRandom(1.0f) > ofMap(dist, 0, terrain_size * 0.9, 1.0, 0.2, true)) continue;

		// --- POWER LAW CLUSTER INTENSITY ---
		// Piles vary wildly in size/density
		float k = 4.0f;
		float intensity = pow(ofRandom(1.0f), k);

		// Lerp based on intensity
		float radius = ofLerp(20.0f, 100.0f, intensity);
		int numTrash = (int)ofLerp(10.0f, 400.0f, intensity);

		for (int i = 0; i < numTrash; i++) {
			float r = sqrt(ofRandom(1.0f)) * radius;
			float theta = ofRandom(TWO_PI);
			float x = cx + r * cos(theta);
			float y = cy + r * sin(theta);

			if (x < 5.0 || x > terrain_size - 15.0 || y < 5.0 || y > terrain_size * 0.91) continue;

			DebrisObject trash;
			float z = getTerrainHeight(x, y) + 1;
			trash.pos = ofVec3f(x, y, z);
			trash.slopeNormal = getTerrainNormal(x, y);

			// Power Law Object Sizes
			float scale = pow(ofRandom(1.0f), 5.0f);
			trash.size = ofVec3f(ofRandom(2, 8), ofLerp(2.0f, 35.0f, scale), ofRandom(1, 5));
			trash.angle = ofRandom(360.0f);

			if (scale < 0.1)
				trash.color = ofColor::gray;
			else
				trash.color = ofColor::tan;

			debrisList.push_back(trash);
		}
	}
}

ofVec3f ofApp::getTerrainNormal(float x, float y) {
	// Sample heights around the point
	float hL = getTerrainHeight(x - 1, y);
	float hR = getTerrainHeight(x + 1, y);
	float hD = getTerrainHeight(x, y - 1);
	float hU = getTerrainHeight(x, y + 1);

	// Create vectors representing the slope
	// Vector pointing Right: (2 units across, change in height)
	ofVec3f meshVecX(2, 0, hR - hL);

	// Vector pointing Up: (2 units up, change in height)
	ofVec3f meshVecY(0, 2, hU - hD);

	// The cross product gives the vector perpendicular to both (The Normal)
	ofVec3f normal = meshVecX.crossed(meshVecY).normalized();

	return normal;
}
