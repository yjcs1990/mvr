// Some type error
#include "Mvria.h"
#include "mvriaUtil.h"
#include <iostream>

void testIntersection(MvrLine *line1, MvrLine *line2, double x, double y,
		      std::string name)
{
  MvrPose pose;

  if (!line1->intersects(line2, &pose) || fabs(pose.getX() - x) > .001 ||
      fabs(pose.getY() - y) > .001)
  {
    printf("%s didn't intersect\n", name.c_str());
    exit(1);
  }

  if (line1->intersects(line1, &pose))
  {
    printf("First line of %s intersected itself\n", name.c_str());
    exit(1);
  }

  if (line2->intersects(line2, &pose))
  {
    printf("Second line of %s intersected itself\n", name.c_str());
    exit(1);
  }
}

void testIntersection(MvrLineSegment *line1, MvrLine *line2, double x, double y,
		      std::string name)
{
  MvrPose pose;

  if (!line1->intersects(line2, &pose) || fabs(pose.getX() - x) > .001 ||
      fabs(pose.getY() - y) > .001)
  {
    printf("%s didn't intersect\n", name.c_str());
    exit(1);
  }

  if (line1->intersects(line1, &pose))
  {
    printf("First line of %s intersected itself\n", name.c_str());
    exit(1);
  }

  if (line2->intersects(line2, &pose))
  {
    printf("Second line of %s intersected itself\n", name.c_str());
    exit(1);
  }
}

void testIntersection(MvrLineSegment *line1, MvrLineSegment *line2, 
		      double x, double y, std::string name)
{
  MvrPose pose;

  if (!line1->intersects(line2, &pose) || fabs(pose.getX() - x) > .001 ||
      fabs(pose.getY() - y) > .001)
  {
    printf("%s didn't intersect\n", name.c_str());
    exit(1);
  }

  if (line1->intersects(line1, &pose))
  {
    printf("First line of %s intersected itself\n", name.c_str());
    exit(1);
  }

  if (line2->intersects(line2, &pose))
  {
    printf("Second line of %s intersected itself\n", name.c_str());
    exit(1);
  }
}

void testPerp(MvrLineSegment *segment, MvrPose perp, MvrPose perpPoint, 
	      std::string name)
{
  MvrPose pose;
  if (!segment->getPerpPoint(perp, &pose) || 
      fabs(pose.getX() - perpPoint.getX()) > .001 ||
      fabs(pose.getY() - perpPoint.getY()) > .001)
  {
    printf("%s wasn't perp but should have been\n", name.c_str());
    exit(1);
  }
}

void testNotPerp(MvrLineSegment *segment, MvrPose perp, std::string name)
{
  MvrPose pose;
  if (segment->getPerpPoint(perp, &pose))
  {
    printf("%s was perp but shouldn't have been, at %.0f %.0f\n", name.c_str(),pose.getX(), pose.getY());
    exit(1);
  }
}

int main(void)
{
  MvrPose pose;

  MvrLine xLine(-2000, 0, 2000, 0);
  MvrLine yLine(100, 500, 100, -500);
  MvrLineSegment xLineSeg(-2000, 0, 2000, 0);
  MvrLineSegment yLineSeg(100, 500, 100, -500);

  // test all our segments
  testIntersection(&xLine, &yLine, 100, 0, "xLine and yLine");
  testIntersection(&xLineSeg, &yLine, 100, 0, "xLineSeg and yLine");
  testIntersection(&yLineSeg, &xLine, 100, 0, "yLineSeg and xLine");
  testIntersection(&xLineSeg, &yLineSeg, 100, 0, "xLineSeg and yLineSeg");
  

  // test the perp on all the segments
  testPerp(&xLineSeg, MvrPose(-2000, 50), MvrPose(-2000, 0), "xLineSeg end1");
  testPerp(&xLineSeg, MvrPose(2000, 50), MvrPose(2000, 0), "xLineSeg end2");
  testPerp(&xLineSeg, MvrPose(357, 50), MvrPose(357, 0), "xLineSeg middle");
  testNotPerp(&xLineSeg, MvrPose(2001, 0), "xLineSeg beyond end2");
  testNotPerp(&xLineSeg, MvrPose(3000, 0), "xLineSeg way beyond end2");
  testNotPerp(&xLineSeg, MvrPose(-2001, 0), "xLineSeg beyond end1");
  testNotPerp(&xLineSeg, MvrPose(-3000, 0), "xLineSeg way beyond end1");
  
  testPerp(&xLineSeg, MvrPose(1000, 0), MvrPose(1000, 0), "xLineSeg point on line");

  printf("All tests completed successfully\n");

  return 0;
}
