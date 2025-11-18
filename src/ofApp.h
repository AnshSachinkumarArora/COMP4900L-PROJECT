#pragma once

#include "ofMain.h"
#include <vector>

// Define type of debris
enum DebrisType {
	ROCK,
	LOG,
	TOWEL
};

//Struct to hold debris info
struct DebrisObject {
	ofVec2f pos;
	DebrisType type;
	ofColor color;
	float radius; //collision radius for distribution
};

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		//Terrain
		ofMesh terrain;
		ofVec2f flow_dir = ofVec2f(0, 1); // Inland flow (from shore at y=1)
		float terrain_size = 800.0f;
		float shoreline_y = terrain_size * 0.9f; // Shore near higher y
		float getTerrainHeight(float x, float y);

		//camera
		ofEasyCam easyCam;

		//DISTRIBUTION LOGIC
		
		//list of debris
		vector<DebrisObject> debrisList;

		//check for point collision
		bool isValidPlacement(ofVec2f p, float r);

		//placement function
		void generateDebrisField();
};
