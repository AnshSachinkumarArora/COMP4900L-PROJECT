#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

	ofSetBackgroundColor(ofColor::black);
	easyCam.setDistance(1000);

	// Generate terrain mesh (grid with noise heights)
	int res = 80; // Higher for smoother, but slower
	terrain.setMode(OF_PRIMITIVE_TRIANGLES);
	for (int iy = 0; iy <= res; ++iy) {
		for (int ix = 0; ix <= res; ++ix) {
			float x = ix * terrain_size / res;
			float y = iy * terrain_size / res;
			float z = getTerrainHeight(x, y);
			terrain.addVertex(ofVec3f(x, y, z));

			// Color: bluer near shore, sandier inland
			float shore_t = (y - shoreline_y) / terrain_size;
			if (shore_t > 0) terrain.addColor(ofColor::fromHex(0xc0c6c9));
			else terrain.addColor(ofColor::fromHex(0x5c5c5c));
		}
	}

	//indices setup
	for (int iy = 0; iy < res; ++iy) {
		for (int ix = 0; ix < res; ++ix) {
			int i = iy * (res + 1) + ix;
			terrain.addTriangle(i, i + 1, i + res + 1);
			terrain.addTriangle(i + res + 1, i + 1, i + res + 2);
		}
	}

	//generated points
	generateDebrisField();
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
	easyCam.begin();

	ofPushMatrix();
	ofTranslate(-terrain_size / 2, -terrain_size / 2);
	terrain.draw();

	//draw different categories of debris
	for (auto& d : debrisList) {
		ofSetColor(d.color);
		ofPushMatrix();
		ofTranslate(d.pos);
		ofRotateZDeg(d.angle);
		ofDrawBox(0, 0, 0, d.size.x, d.size.y, d.size.z);
		ofPopMatrix();
	}

	ofPopMatrix();

	easyCam.end();
}

float ofApp::getTerrainHeight(float x, float y) {
	float slope = (y - shoreline_y) / terrain_size * 30.0f; // Gentle rise inland
	float noise1 = ofNoise(x * 0.005f, y * 0.005f) * 10.0f; // Large waves
	float noise2 = ofNoise(x * 0.02f, y * 0.02f) * 5.0f; // Small details
	return slope + noise1 + noise2;
}

bool ofApp::isValidPlacement(ofVec2f p, float r) {
	//bounds check
	if (p.x < 5.0 || p.x > terrain_size-15.0 || p.y < 5.0 || p.y > terrain_size*0.95) return false;

	//check against other debris (brute force since there aren't going to be an enormous amount of objects)
	for (auto& d : debrisList) {
		float dist = p.distance(d.pos);
		//min dist is sum of radii
		float reductionFactor = 0.0f;
		if (r <= 3.0f) reductionFactor = 10.0f;
		if (dist < (r + d.radius - reductionFactor)) return false;
	}
	return true;
}

void ofApp::generateDebrisField() {
	debrisList.clear();

	//setting cluster centers
	vector<ofVec3f> clusterCenters;

	int numCenters = 300;

	int attempts = 0;
	while (clusterCenters.size() < numCenters && attempts < 10000) {
		attempts++;
		float x = ofRandom(terrain_size);
		float y = ofRandom(terrain_size);

		//shoreline gradient
		float distFromShore = abs(y - shoreline_y);
		float prob = ofMap(distFromShore, 0, terrain_size * 0.9, 1.0, 0.2, true);

		if (ofRandom(1.0) < prob) {
			float z = getTerrainHeight(x, y);
			clusterCenters.push_back(ofVec3f(x, y, z));
		}
	}

	//fill clusters (apply power law to pile size)
	for (const auto& center : clusterCenters) {
		//power law magnitude to determine size of pile
		float k = 4.0;
		float intensity = pow(ofRandom(0, 1), k);

		//use intensity for radius and count
		//pile sizes
		float radius = ofLerp(20.0, 100.0, intensity);
		int numTrash = ofLerp(10, 400, intensity);

		for (int i = 0; i < numTrash; i++) {

			// Matérn Placement: Uniformly fill the circle
			float r = sqrt(ofRandom(1.0)) * radius;
			float theta = ofRandom(TWO_PI);

			float tx = center.x + r * cos(theta);
			float ty = center.y + r * sin(theta);

			// Bounds check
			if (tx < 5.0 || tx > terrain_size - 15.0 || ty < 5.0 || ty > terrain_size * 0.91) continue;

			DebrisObject trash;
			float z = getTerrainHeight(tx, ty) + 1; // Sit on top of terrain
			trash.pos = ofVec3f(tx, ty, z);

			// Object Visuals
			// Since it's "small trash", we keep size variation subtle
			trash.size = ofVec3f(ofRandom(2, 5), ofRandom(5, 15), ofRandom(1, 3));

			// Piles are chaotic, so random rotation is best
			trash.angle = ofRandom(360);

			// Color Variation
			float colRnd = ofRandom(1.0);
			if (colRnd > 0.6)
				trash.color = ofColor::gray;
			else if (colRnd > 0.3)
				trash.color = ofColor::tan;
			else
				trash.color = ofColor::fromHex(0x444444); // Dark wet debris

			debrisList.push_back(trash);
		}
	}
}

void ofApp::keyPressed(int key) {
	generateDebrisField();
	draw();
}
