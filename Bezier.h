// Bezier Class by Ryan Stachura
// CS 450/550, OSU, 11/21/2021
#pragma once
#include <vector>
typedef std::vector<float> vFloat;
typedef std::vector<vFloat> vvFloat;

const int	NUM_LINE_STRIPS			= 100;				// the smoothness (higher => smoother)
const float	CURVE_COLOR[3]			= { 0.0, 1.0, 0.0 };// Green
const float CONTROL_LINE_COLOR[3]	= { 1.0, 0.5, 0.0 };// Orange
const float CONTROL_POINT_COLOR[3]	= { 1.0, 1.0, 0.0 };// Yellow

float	randfInRange(float);
int		factorial(int);
float	binomial(float, float);
float	calcBezierAtT(float, vFloat);

// A list of control points for a Bezier curve
class ControlPoints {
private:
	int count;
	float range;

public:
	vFloat xs, ys, zs;
	// Default constuctor to initialize fields
	ControlPoints() {
		this->xs = this->ys = this->zs = {};
		this->count = 0;
		this->range = 10.0;
	}

	// Split added point into axis vectors
	void add(float x, float y, float z) {
		this->xs.push_back(x);
		this->ys.push_back(y);
		this->zs.push_back(z);
		this->count++;
	}

	// Empty vectors
	void clear() {
		this->xs.clear();
		this->ys.clear();
		this->zs.clear();
		this->count = 0;
	}

	// Randomizes the control points in [-range, range]
	void randomizePoints() {
		for (int i = 0; i < this->count; i++) {
			this->xs[i] = randfInRange(this->range);
			this->ys[i] = randfInRange(this->range);
			this->zs[i] = randfInRange(this->range);
		}
	}

	// Getters
	int getCount()		{ return this->count; }
	float getRange()	{ return this->range; }
	vFloat getXs()		{ return this->xs; }
	vFloat getYs()		{ return this->ys; }
	vFloat getZs()		{ return this->zs; }
};

// A Bezier curve
class Bezier {
private:
	int degree;
	bool doControlLines, doControlPoints;

public:
	ControlPoints controlPoints;
	// Default constructor to intialize fields
	Bezier() {
		this->controlPoints = ControlPoints();
		this->degree = 0;
		this->doControlLines = this->doControlPoints = false;
	}

	// Add a point and adjust degree
	void addPoint(float x, float y, float z) {
		this->controlPoints.add(x, y, z);
		this->degree = this->controlPoints.getCount() - 1;
	}

	// Modify the x/y/z-points of a point
	void modifyPoint(int index, float x, float y, float z) {
		this->controlPoints.xs[index] = x;
		this->controlPoints.ys[index] = y;
		this->controlPoints.zs[index] = z;
	}

	// Modify the z-point of a point
	void modifyPointZ(int index, float z) {
		this->controlPoints.zs[index] = z;
	}

	// Draws the curve
	void draw() {
		float bxt, byt, bzt, t, count, colorToRestore[4];
		vFloat curvexs, curveys, curvezs;

		count	= this->controlPoints.getCount();
		curvexs = this->controlPoints.getXs();
		curveys = this->controlPoints.getYs();
		curvezs = this->controlPoints.getZs();

		// drawing the curve
		glColor3fv(CURVE_COLOR);
		glBegin(GL_LINE_STRIP);
		for (int i = 0; i <= NUM_LINE_STRIPS; i++) {
			t	= (float)i / (float)NUM_LINE_STRIPS;
			bxt	= calcBezierAtT(t, curvexs);
			byt = calcBezierAtT(t, curveys);
			bzt = calcBezierAtT(t, curvezs);

			glVertex3f(bxt, byt, bzt);
		}
		glEnd();


		glGetFloatv(GL_CURRENT_COLOR, colorToRestore);

		// show the control lines
		if (this->doControlLines) {
			float x, y, z;

			glColor3fv(CONTROL_LINE_COLOR);
			glBegin(GL_LINE_STRIP);
			for (unsigned int i = 0; i < count; i++) {
				x = curvexs[i];
				y = curveys[i];
				z = curvezs[i];
				glVertex3f(x, y, z);
			}
			glEnd();
		}

		// show the control points
		if (this->doControlPoints) {
			float x, y, z;

			glColor3fv(CONTROL_POINT_COLOR);
			for (unsigned int i = 0; i < count; i++) {
				x = curvexs[i];
				y = curveys[i];
				z = curvezs[i];
				glPushMatrix();
				glTranslatef(x, y, z);
				glutSolidSphere(0.1, 10, 10);
				glPopMatrix();
			}
		}
	
		glColor4fv(colorToRestore);
	}

	// Enable/disable show control lines
	void showControlLines(bool b) { this->doControlLines = b; }

	// Enable/disable show control points
	void showControlPoints(bool b) { this->doControlPoints = b; }

	// Randomize points
	void randomize() { this->controlPoints.randomizePoints(); }

	// Randomize points and linearly interpolate to them over time
	void randomizeAndInterpolate(float durationSeconds) {
		ControlPoints cps		= this->controlPoints;	// The current control points
		vFloat xs				= cps.getXs();			// The current control point's x-values
		vFloat ys				= cps.getYs();			// The current control point's y-values
		vFloat zs				= cps.getZs();			// The current control point's z-values

		ControlPoints nextCps;							// The control points to interpolate towards
		vvFloat dvs	= {};								// The direction vectors

		// Generate new random points then a list of each direction vector
		{
			vFloat dv = {};
			float x, y, z, range = cps.getRange();
			for (int i = 0; i < cps.getCount(); i++) {
				// create new random points and add to new control point list
				x = randfInRange(range);
				y = randfInRange(range);
				z = randfInRange(range);
				nextCps.add(x, y, z);

				// create direction vector for this point pair
				dv.push_back(x - xs[i]);
				dv.push_back(y - ys[i]);
				dv.push_back(z - zs[i]);
				dvs.push_back(dv);
				dv.clear();

				// print new points
				//fprintf(stderr, "p%d: (%+f, %+f, %+f)\n", i, x, y, z);
			}
			//printf("=======================================\n");
		}

		// Linearly interpolate points over time
		{
			ControlPoints iCps;
			int msStart, msNow;
			float t = 0.0;
			msStart = msNow = glutGet(GLUT_ELAPSED_TIME);
			while (t < 1.0) {
				t = (float)(msNow - msStart) / (durationSeconds * (float)1000);
				//fprintf(stderr, "t = %f\n", t);

				// Calculate curve at current interpolation percentage
				for (int i = 0; i < cps.getCount(); i++) iCps.add(
					xs[i] + (t * dvs[i][0]),
					ys[i] + (t * dvs[i][1]),
					zs[i] + (t * dvs[i][2])
				);
				this->controlPoints = iCps;
				this->draw();

				iCps.clear();
				msNow = glutGet(GLUT_ELAPSED_TIME);
			}

			// Set new control points
			this->controlPoints = nextCps;
		}
	}

	// Getters
	ControlPoints getControlPoints() { return this->controlPoints; }
	int getDegree() { return this->degree; }
};

// Random float in range [-r, r]
float randfInRange(float r) {
	return ( static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (r * 2.0))) ) - r;
}

// n!
int factorial(int n) {
	if (n > 1) return n * factorial(n - 1);
	return 1;
}

// binomial (n, k)
float binomial(float n, float k) {
	return factorial(n) / (factorial(k) * factorial(n - k));
}

// nth order Bezier curve at t
float calcBezierAtT(float t, vFloat ps) {
	float bt, n, size;

	// bound control
	if (t < 0.0)		t = 0.0;
	else if (t > 1.0)	t = 1.0;

	// set the degree
	size = ps.size();
	n = size - 1;

	// calculate bt
	bt = 0.0;
	for (int i = 0; i < size; i++)
		bt += binomial(n, i) * pow(t, i) * pow(1 - t, n - i) * ps[i];

	return bt;
}
