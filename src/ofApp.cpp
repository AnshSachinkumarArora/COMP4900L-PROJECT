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
			if (shore_t > 0) terrain.addColor(ofColor::navy);
			else terrain.addColor(ofColor::sandyBrown);
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
		float z = getTerrainHeight(d.pos.x, d.pos.y) + 5;
		ofPushMatrix();
		ofTranslate(d.pos.x, d.pos.y, z);
		ofRotateZDeg(d.angle);

		if (d.type == ROCK) ofDrawSphere(0, 0, 0, d.radius * 0.5);
		else if (d.type == LOG) ofDrawBox(0, 0, 0, d.radius/4, d.radius*4, d.radius/4);
		else ofDrawCircle(0, 0, 0, d.radius);
		ofPopMatrix();
	}

	ofPopMatrix();

	easyCam.end();
}

float ofApp::getTerrainHeight(float x, float y) {
	float slope = (y - shoreline_y) / terrain_size * 20.0f; // Gentle rise inland
	float noise1 = ofNoise(x * 0.005f, y * 0.005f) * 10.0f; // Large waves
	float noise2 = ofNoise(x * 0.02f, y * 0.02f) * 5.0f; // Small details
	return slope + noise1 + noise2;
}

bool ofApp::isValidPlacement(ofVec2f p, float r) {
	//bounds check
	if (p.x < 15.0 || p.x > terrain_size-15.0 || p.y < 15.0 || p.y > terrain_size*0.85) return false;

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

	//phase 1: rock placement
	//rocks are heavy so they stay even in the high flow areas
	int targetRocks = 50;
	//try 1000 times
	for (int i = 0; i < 1000; i++) {
		if (debrisList.size() >= targetRocks) break;

		ofVec2f pos(ofRandom(terrain_size), ofRandom(terrain_size));
		float r = ofRandom(15.0f, 40.0f); //large rock radius

		if (isValidPlacement(pos, r)) {
			DebrisObject rock;
			rock.pos = pos;
			rock.type = ROCK;
			rock.radius = r;
			rock.color = ofColor::grey;
			rock.angle = ofRandom(360);
			debrisList.push_back(rock);
		}
	}

	//phase 2: log placement
	//tusnami flow is strongest in the middle so light/medium objects are pushed to the side
	int targetLogs = 100;
	float centerLine = terrain_size / 2.0;
	float flowChannelWidth = 400.0f;

	for (int i = 0; i < 10000; i++) {
		if (debrisList.size() > targetLogs + targetRocks) break;

		ofVec2f pos(ofRandom(terrain_size), ofRandom(terrain_size));
		float r = ofRandom(8.0f, 20.0f); //medium radius

		//distance from center flow and probability of placement
		float distFromCenter = abs(pos.x - centerLine);
		float channelProb = ofMap(distFromCenter, 0, flowChannelWidth, 0.0, 1.0, true);

		//distance from shore and density near shoreline
		float distFromShore = abs(pos.y - shoreline_y);
		float shoreProb = ofMap(distFromShore, 0, terrain_size*0.8, 1.0, 0.0, true);

		//final probability for log placement
		float finalProb = channelProb * shoreProb;

		if (ofRandom(1.0) < finalProb) {
			if (isValidPlacement(pos, r)) {
				DebrisObject log;
				log.pos = pos;
				log.type = LOG;
				log.radius = r;
				log.color = ofColor::brown;
				//add anisotropy to log angle
				log.angle = ofRandom(-20, 20);
				debrisList.push_back(log);
			}
		}
	}

	//phase 3: beach towel/human light object placement
	//light objects like towels don't get thrown randomly, they get snagged around heavy objects
	vector<int> rockIndices;
	for (int i = 0; i < debrisList.size(); i++) {
		if (debrisList[i].type == ROCK) rockIndices.push_back(i);
	}

	for (int i : rockIndices) {
		int numTowels = ofRandom(2, 5); //randomly between 2-5 towels per rock

		for (int t = 0; t < numTowels; t++) {
			ofVec2f rockPos = debrisList[i].pos;

			//pick spot close to rock
			float angle = ofRandom(TWO_PI);
			float dist = ofRandom(10.0, 15.0);
			ofVec2f offset(cos(angle) * dist, sin(angle) * dist);
			ofVec2f pos = rockPos + offset;

			float r = 3.0f;

			//verify to check for overlap and add towel
			if (isValidPlacement(pos, r)) {
				DebrisObject towel;
				towel.pos = pos;
				towel.type = TOWEL;
				towel.radius = r;
				towel.color = ofColor::white;
				towel.angle = ofRandom(360);
				debrisList.push_back(towel);
			}
		}
	}
}

void ofApp::keyPressed(int key) {
	generateDebrisField();
	draw();
}
