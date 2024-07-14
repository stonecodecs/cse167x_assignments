#include "WorkingScene.h"

// This file includes the basic functions that your program must fill in.  
// Note that there are several helper functions from Curve.cpp that can be used!
// In particular, take a look at moveActivePoint, connectTheDots, drawLine, etc.

// user defined
float float_divide(int x, int dim) {
	float x_norm = static_cast<float>(x) / static_cast<float>(dim);
	return x_norm;
}

// What happens when you drag the mouse to x and y?  
// In essence, you are dragging control points on the curve.
void WorkingScene::drag(int x, int y) {
	/* YOUR CODE HERE */
	//you must figure out how to transform x and y so they make sense
	//update oldx, and oldy
	//make sure scene gets redrawn
	if (theOnlyCurve != NULL) {
		float dx_norm = float_divide(x - oldx, Scene::width);
		float dy_norm = float_divide(oldy - y, Scene::height);
		theOnlyCurve->moveActivePoint(dx_norm, dy_norm);
		Scene::oldx = x;
		Scene::oldy = y;
		glutPostRedisplay();
	}

}

// Mouse motion.  You need to respond to left clicks (to add points on curve) 
// and right clicks (to delete points on curve) 
void WorkingScene::mouse(int button, int state, int x, int y) {
	// theOnlyCurve is the current type of curve being drawn. It is created in Scene.cpp.
	if (theOnlyCurve && state == GLUT_DOWN) {
		if (button == GLUT_LEFT_BUTTON) {
			if (Scene::width > 0.0 && Scene::height > 0.0) {
				// normalize coordinates to view in space
				float x_norm = float_divide(x, Scene::width);
				float y_norm = float_divide(Scene::height - y, Scene::height);
				// printf("x,y@(x: %d, y: %d)\n", x, y);
				// printf("linenorms@(x: %f, y: %f)\n", x_norm, y_norm);
				theOnlyCurve->addPoint(x_norm, y_norm);
				// printf("create@(button: %d, state: %d)\n", button, state);
				// button: {0: LMB, 1: MMB, 2:RMB}, state {0:pressed, 1:released}
				theOnlyCurve->getNumPoints();
				theOnlyCurve->draw(Scene::levelOfDetail);
			}
		}
		if (button == GLUT_RIGHT_BUTTON) {	
			theOnlyCurve->deleteActivePoint();
			theOnlyCurve->draw(Scene::levelOfDetail);
		}
	}

	/* YOUR CODE HERE */
	//update oldx, and oldy
	//make sure scene gets redrawn
	Scene::oldx = x;
	Scene::oldy = y;
	glutPostRedisplay();
}



#include "Bezier.h"

// Bezier drawing function.  This is by deCasteljau or equivalent algorithm. 
// It should support Bezier curves of arbitrary degree/order.
void Bezier::draw(int levelOfDetail) {

	connectTheDots();
	int i, j, k;
	Pvector::iterator p, next;

	/* YOUR CODE HERE */
	float u;
	std::vector<float> u_x(0);
	std::vector<float> u_y(0);

	// for ever control point in curve
	// compute f(u) for t=0...1 separated by LoD)
	for (int lvl = levelOfDetail; lvl >= 0; lvl--) {
		u = float_divide(lvl, levelOfDetail);
		Pvector blossoms = points; // vector 

		if (blossoms.size() > 0) {
			for (int i = 0; i < points.size() - 1; i++) { // # levels
				for (int j = 0; j < points.size() - 1 - i; j++) {
					blossoms[j].x = (blossoms[j].x * (1 - u)) + (blossoms[j + 1].x * u);
					blossoms[j].y = (blossoms[j].y * (1 - u)) + (blossoms[j + 1].y * u);
				}
			}
			// into final f(u).{x,y} 
			u_x.push_back(blossoms[0].x);
			u_y.push_back(blossoms[0].y);
		}
	}

	// draw f(u) lines
	for (int i = 0; i < (int)u_x.size() - 1; i++) {
		Scene::theOnlyCurve->drawLine(u_x[i], u_y[i], u_x[i + 1], u_y[i + 1]);
	}
}



#include "Bspline.h"

// The B-Spline drawing routine.  
// Remember to call drawSegment (auxiliary function) for each set of 4 points.
void Bspline::draw(int levelOfDetail) {
	connectTheDots();
	/* YOUR CODE HERE */ 
	if (points.size() >= 4) {
		for (Pvector::iterator p0 = points.begin(); p0 != points.end() - 1; p0++) {
			// for each 4 points
			Pvector::iterator p1 = p0 + 1;
			Pvector::iterator p2 = p1 + 1;
			Pvector::iterator p3 = p2 + 1;
			if (p1 == points.end() || p2 == points.end() || p3 == points.end()) { return; }
			drawSegment(p0, p1, p2, p3, levelOfDetail);
		}
	}
	
}

void Bspline::drawSegment(Pvector::iterator p1, Pvector::iterator p2, Pvector::iterator p3, Pvector::iterator p4, int levelOfDetail) {
	float x, y;

	/* YOUR CODE HERE */
	//draw segment
		
	// last "third" for each line generated by each pair of points 
	// first third not used in computation
	float third1_2_x = (p1->x * 1.0f / 3.0f) + (p2->x * 2.0f / 3.0f);
	float third1_2_y = (p1->y * 1.0f / 3.0f) + (p2->y * 2.0f / 3.0f);

	float third2_1_x = (p2->x * 2.0f / 3.0f) + (p3->x * 1.0f / 3.0f);
	float third2_1_y = (p2->y * 2.0f / 3.0f) + (p3->y * 1.0f / 3.0f);
	
	float third2_2_x = (p2->x * 1.0f / 3.0f) + (p3->x * 2.0f / 3.0f);
	float third2_2_y = (p2->y * 1.0f / 3.0f) + (p3->y * 2.0f / 3.0f);
	
	float third3_1_x = (p3->x * 2.0f / 3.0f) + (p4->x * 1.0f / 3.0f);
	float third3_1_y = (p3->y * 2.0f / 3.0f) + (p4->y * 1.0f / 3.0f);
	// last third not used in computation

	// half for 2nd level
	float level2L_x = (third1_2_x / 2.0f) + (third2_1_x / 2.0f);
	float level2L_y = (third1_2_y / 2.0f) + (third2_1_y / 2.0f);
	float level2R_x = (third2_2_x / 2.0f) + (third3_1_x / 2.0f);
	float level2R_y = (third2_2_y / 2.0f) + (third3_1_y / 2.0f);

	// create a 'local' Bezier local to each set of 4 points in each spline (L,2_1,2_2,R)
	// reuse from Bezier1
	float u;
	Point knot;
	std::vector<float> u_x(0);
	std::vector<float> u_y(0);
	Pvector localpts = { 
		Point(level2L_x, level2L_y),
		Point(third2_1_x, third2_1_y),
		Point(third2_2_x, third2_2_y),
		Point(level2R_x, level2R_y)
	};

	// for ever control point in curve
	// compute f(u) for t=0...1 separated by LoD)
	for (int lvl = levelOfDetail; lvl >= 0; lvl--) {
		u = float_divide(lvl, levelOfDetail);
		Pvector blossoms = localpts; // vector 

		if (blossoms.size() > 0) {
			for (int i = 0; i < localpts.size() - 1; i++) { // # levels
				for (int j = 0; j < localpts.size() - 1 - i; j++) {
					blossoms[j].x = (blossoms[j].x * (1 - u)) + (blossoms[j + 1].x * u);
					blossoms[j].y = (blossoms[j].y * (1 - u)) + (blossoms[j + 1].y * u);
				}
			}
			// into final f(u).{x,y} 
			u_x.push_back(blossoms[0].x);
			u_y.push_back(blossoms[0].y);
			if (u > 0.9999) { // epsilon for floating point; really, just a check for 1
				// knot at the end
				knot = Point(blossoms[0].x, blossoms[0].y);
			}
		}
	}

	// draw f(u) lines
	for (int i = 0; i < (int)u_x.size() - 1; i++) {
		Scene::theOnlyCurve->drawLine(u_x[i], u_y[i], u_x[i + 1], u_y[i + 1]);
	}
	knot.draw();
	 
	// update knot location (interpolate these half way)
	//then create a Point to be drawn where the knot should be
	//Point p(x, y);
	//p.draw();
}

#include "Bezier2.h"

//This function is provided to aid you.
//It should be used in the spirit of recursion, though you may choose not to.
//This function takes an empty vector of points, accum
//It also takes a set of control points, pts, and fills accum with
//the control points that correspond to the next level of detail.
void accumulateNextLevel(Pvector* accum, Pvector pts) {
	if (pts.empty()) return; 
	accum->push_back(*(pts.begin()));
	if (pts.size() == 1) return;
	for (Pvector::iterator it = pts.begin(); it != pts.end() - 1; it++) {
		/* YOUR CODE HERE  (only one to three lines)*/
		Pvector::iterator next = it + 1;
		it->x = (it->x + next->x) / 2.0f;
		it->y = (it->y + next->y) / 2.0f;
	}
	//save the last point
	Point last = *(pts.end()-1);
	pts.pop_back();
	//recursive call
	accumulateNextLevel(accum, pts);
	accum->push_back(last);
}


// The basic draw function for Bezier2.  Note that as opposed to Bezier, 
// this draws the curve by recursive subdivision.  So, levelofdetail 
// corresponds to how many times to recurse.  
void Bezier2::draw(int levelOfDetail) {
	//This is just a trick to find out if this is the top level call
	//All recursive calls will be given a negative integer, to be flipped here
	if (levelOfDetail > 0) {
		connectTheDots();
	} else {
		levelOfDetail = -levelOfDetail;
	}

	//Base case.  No more recursive calls.
	if (levelOfDetail <= 1) {
		if (points.size() >= 2) {
			printf("level1");
			for (Pvector::iterator it = points.begin(); it != points.end() - 1; it++) {
				/* YOUR CODE HERE */
				// just a simple connection between control points @ base case level
				Pvector::iterator next = it + 1;
				Scene::theOnlyCurve->drawLine(it->x, it->y, next->x, next->y);
				// printf("(%f,%f)point\n", p.x, p.y);
			}
		}
	} else {
		Pvector* accum = new Pvector();
		Bezier2 left, right;

		//add the correct points to 'left' and 'right'.
		//You may or may not use accum as you see fit.
		/* YOUR CODE HERE */

		if (points.size() > 0) {
			accumulateNextLevel(accum, points);

			int mid = accum->size() / 2;
			left.points.assign(accum->begin(), accum->begin() + mid + 1);
			right.points.assign(accum->begin() + mid, accum->end());

			left.draw(1 - levelOfDetail);
			right.draw(1 - levelOfDetail);
			delete accum;
		}
	}
}
