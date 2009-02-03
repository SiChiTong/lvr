#include "BaseVertex.h"
#include "Matrix4.h"

BaseVertex::BaseVertex(){
  x = y = z = 0.0;
}

BaseVertex::BaseVertex(float _x, float _y, float _z){
  x = _x;
  y = _y;
  z = _z;
}

BaseVertex::BaseVertex(const BaseVertex &o){
  x = o.x;
  y = o.y;
  z = o.z;
}

float BaseVertex::length(){
  return sqrt(x * x + y * y + z * z);
}

BaseVertex BaseVertex::cross(const BaseVertex o) const{

  float tx = y * o.z - z * o.y;
  float ty = z * o.x - x * o.z;
  float tz = x * o.y - y * o.x;

  return BaseVertex(tx, ty, tz);

}

void BaseVertex::crossTo(const BaseVertex o){

  x = y * o.z - z * o.y;
  y = z * o.x - x * o.z;
  x = x * o.y - y * o.x;

}

void BaseVertex::rotate(Matrix4 m){

	x = x * m[0 ] + y * m[1 ] + z * m[2 ];
	y = x * m[4 ] + y * m[5 ] + z * m[6 ];
	z = z * m[8 ] + y * m[9 ] + z * m[10];

}

float BaseVertex::operator[](const int i){

  float ret = 0.0;

  switch(i){

  case 0:
    ret = x;
    break;

  case 1:
    ret = y;
    break;

  case 2:
    ret = z;
    break;

  default:
    cout << "BaseVertex: Warning: Access index out of range." << endl;

  }

  return ret;

}

float BaseVertex::operator*(const BaseVertex v) const{
  return x * v.x + y * v.y + z * v.z;
}

BaseVertex BaseVertex::operator*(const float s) const{
  return BaseVertex(x * s, y * s, z * s);
}



BaseVertex BaseVertex::operator+(const BaseVertex v) const{
  return BaseVertex(x + v.x, y + v.y, z + v.z);
}

BaseVertex BaseVertex::operator-(const BaseVertex v) const{
  return BaseVertex(x - v.x, y - v.y, z - v.z);
}

void BaseVertex::operator-=(const BaseVertex v){
  x *= v.x;
  y *= v.y;
  z *= v.z;
}

void BaseVertex::operator+=(const BaseVertex v){
  x += v.x;
  y += v.y;
  z += v.z;
}

void BaseVertex::operator*=(const float s){
  x *= s;
  y *= s;
  z *= s;
}

void BaseVertex::operator/=(const float s){
  if(s != 0) x /= x; else x = 0.0;
  if(s != 0) y /= y; else y = 0.0;
  if(s != 0) z /= z; else z = 0.0;
}

bool BaseVertex::operator==(const BaseVertex &other) const{
	return fabs(x - other.x) < BaseVertex::epsilon &&
		   fabs(y - other.y) < BaseVertex::epsilon &&
		   fabs(z - other.z) < BaseVertex::epsilon;
}

bool BaseVertex::operator!=(const BaseVertex &other) const{
	return !(*this == other);
}
