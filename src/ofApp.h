#pragma once

#include "ofMain.h"
#include <vector>

// Define type of debris
enum DebrisType {
	SMALL_DEBRIS
};

//Struct to hold debris info
struct DebrisObject {
	ofVec3f pos;
    ofColor color;
    ofVec3f size; 
    float angle;      // Rotation around its own axis (Yaw)
    ofVec3f slopeNormal; // The angle of the ground beneath it
};

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		void keyPressed(int key);

		//Terrain
		ofMesh terrain;
		ofVec2f flow_dir = ofVec2f(0, 1); // Inland flow (from shore at y=1)
		float terrain_size = 800.0f;
		float shoreline_y = terrain_size * 0.9f; // Shore near higher y
		float getTerrainHeight(float x, float y);
		ofVec3f getTerrainNormal(float x, float y);

		//camera
		ofEasyCam easyCam;
		
		//list of debris
		vector<DebrisObject> debrisList;

		//check for point collision
		bool isValidPlacement(ofVec2f p, float r);

		// Mode Tracking
		int currentMode = 3; // Start with the best one (3)

		// The 3 Distribution Algorithms
		void generateJustPowerLaw(); // Mode 1
		void generateJustMatern(); // Mode 2
		void generateMaternPowerLaw(); // Mode 3 (The Hybrid)

		// Helper to run the correct function based on mode
		void generateDistribution();
};
