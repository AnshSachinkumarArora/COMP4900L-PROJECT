#pragma once

#include "ofMain.h"
#include <vector>

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		/*
		ofMesh terrainMesh;
		int terrainWidth = 500;
		int terrainHeight = 500;
		int terrainGridStep = 10;
		*/

		//Terrain
		ofMesh terrain;
		ofVec2f flow_dir = ofVec2f(0, 1); // Inland flow (from shore at y=0)
		float terrain_size = 800.0f;
		float shoreline_y = terrain_size * 0.9f; // Shore near higher y
		float getTerrainHeight(float x, float y);

		/*
		//noise
		float noiseFrequency = 0.01;
		float time = 0.0;
		float timeSpeed = 0.02;
		*/

		//camera
		ofEasyCam easyCam;

		//objects

		//point placement
		vector<ofVec2f> points;
		vector<ofVec2f> generatePointSamples(float width, float height, float r, float k = 30);
};
