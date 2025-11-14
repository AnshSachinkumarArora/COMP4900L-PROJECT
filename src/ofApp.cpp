#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	//OLD METHOD FOR DRAWING SHORE
	/* terrainMesh.setMode(OF_PRIMITIVE_TRIANGLES);

	//generate vertices
	for (int y = 0; y < terrainHeight; y++) {
		for (int x = 0; x < terrainWidth; x++) {
			float noiseVal = ofNoise(x * noiseFrequency, y * noiseFrequency);

			//noise for height
			float z = ofMap(noiseVal, 0, 1, -30, 30);
			terrainMesh.addVertex(glm::vec3(x - terrainWidth / 2, y - terrainHeight / 2, z));

			//noise for color
			ofColor b = ofColor::blue;
			ofColor w = ofColor::white;
			terrainMesh.addColor(b.lerp(w, noiseVal));
		}
	}

	//generate indices to connect vertices
	int gridW = terrainWidth / terrainGridStep;
	int gridH = terrainHeight / terrainGridStep;
	for (int y = 0; y < gridW - 1; y++) {
		for (int x = 0; x < gridH - 1; x++) {
			int i1 = (y * gridW) + x; // Top-left
			int i2 = (y * gridW) + (x + 1); // Top-right
			int i3 = ((y + 1) * gridW) + x; // Bottom-left
			int i4 = ((y + 1) * gridW) + (x + 1); // Bottom-right

			// Triangle 1 (Top-left, Top-right, Bottom-left)
			terrainMesh.addIndex(i1);
			terrainMesh.addIndex(i2);
			terrainMesh.addIndex(i3);

			// Triangle 2 (Top-right, Bottom-right, Bottom-left)
			terrainMesh.addIndex(i2);
			terrainMesh.addIndex(i4);
			terrainMesh.addIndex(i3);
		}
	}*/

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
			if (shore_t > 0) {
				terrain.addColor(ofColor::navy);
			} else {
				terrain.addColor(ofColor::sandyBrown);
			}
			/* ofColor b = ofColor::blue;
			ofColor w = ofColor::brown;
			terrain.addColor(w.lerp(b, shore_t));*/
		}
	}
	for (int iy = 0; iy < res; ++iy) {
		for (int ix = 0; ix < res; ++ix) {
			int i = iy * (res + 1) + ix;
			terrain.addTriangle(i, i + 1, i + res + 1);
			terrain.addTriangle(i + res + 1, i + 1, i + res + 2);
		}
	}

	//generated points
	points = generatePointSamples(terrain_size, shoreline_y-20, 10.0);
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
	easyCam.begin();
	terrain.draw();
	ofSetColor(ofColor::red);
	for (int i = 0; i < points.size(); i++) {
		ofDrawCircle(points[i].x, points[i].y, 1);
	}
	easyCam.end();
}

float ofApp::getTerrainHeight(float x, float y) {
	float slope = (y - shoreline_y) / terrain_size * 20.0f; // Gentle rise inland
	float noise1 = ofNoise(x * 0.005f, y * 0.005f) * 10.0f; // Large waves
	float noise2 = ofNoise(x * 0.02f, y * 0.02f) * 5.0f; // Small details
	return slope + noise1 + noise2;
}

vector<ofVec2f> ofApp::generatePointSamples(float width, float height, float r, float k) {
	vector<ofVec2f> samples;
	vector<int> active_list;
	float cell_size = r / sqrt(2.0f);
	int grid_cols = ceil(width / cell_size);
	int grid_rows = ceil(height / cell_size);
	vector<vector<int>> grid(grid_rows, vector<int>(grid_cols, -1));

	ofVec2f initial(ofRandom(width), ofRandom(height));
	samples.push_back(initial);
	int row = floor(initial.y / cell_size);
	int col = floor(initial.x / cell_size);
	grid[row][col] = 0;
	active_list.push_back(0);

	// Step 2: Process active list
	while (!active_list.empty()) {
		size_t rand_idx = size_t(ofRandom(active_list.size()));
		int i = active_list[rand_idx];
		ofVec2f xi = samples[i];
		bool found = false;

		for (int attempt = 0; attempt < k; ++attempt) {
			// Generate candidate in annulus r to 2r
			float theta = ofRandom(TWO_PI);
			float dist = ofRandom(r, 2 * r);
			ofVec2f candidate = xi + ofVec2f(dist * cos(theta), dist * sin(theta));

			// Check bounds
			if (candidate.x < 0 || candidate.x >= width || candidate.y < 0 || candidate.y >= height) continue;

			// Check nearby samples
			int crow = floor(candidate.y / cell_size);
			int ccol = floor(candidate.x / cell_size);
			bool valid = true;
			int search_radius = 2; // Sufficient for cell_size = r/sqrt(2)
			for (int dy = -search_radius; dy <= search_radius && valid; ++dy) {
				for (int dx = -search_radius; dx <= search_radius && valid; ++dx) {
					int nrow = crow + dy;
					int ncol = ccol + dx;
					if (nrow >= 0 && nrow < grid_rows && ncol >= 0 && ncol < grid_cols) {
						int sidx = grid[nrow][ncol];
						if (sidx != -1 && samples[sidx].distance(candidate) < r) {
							valid = false;
						}
					}
				}
			}

			if (valid) {
				int newidx = samples.size();
				samples.push_back(candidate);
				grid[crow][ccol] = newidx;
				active_list.push_back(newidx);
				found = true;
				break;
			}
		}

		if (!found) {
			// Remove from active list (swap and pop)
			active_list[rand_idx] = active_list.back();
			active_list.pop_back();
		}
	}

	return samples;
}
