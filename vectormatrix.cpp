#include "stdafx.h"
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <memory.h>
#include <math.h>
// #include "image_add_win.h"
//#include "imagedata.h"
//#include "measurements.h"
#include "vectormatrix.h"

using namespace image_add;

vector image_add::operator*(double l, vector v) { 
	return vector(v.x*l,v.y*l); 
}

vector operator-(vector v) { 
	return vector(-v.x,-v.y); 
}

int vector::on_line(vector v1, vector v2, double tol) {
	vector m,n,d;
	double l,u,v;
	n.x=x-v1.x;
	n.y=y-v1.y;
	m.x=x-v2.x;
	m.y=y-v2.y;
	d=v2-v1;l=d.len;
	if (l <= 0) {
		if (n.len <= tol)
			return 1;
		return 0;
	}		
	if (n.len <= tol) return 1;
	if (m.len <= tol) return 1;
	u=n*d/l;
	v=n^d/l;
	if ((u >=0) && (u <= l) && (fabs(v) <= tol)) return 1;
	return 0;
}

double image_add::angulardiff(double p1, double p2) {
	double diff=p1-p2;
	while (diff > M_PI) diff-=2*M_PI;
	while (diff <= -M_PI) diff+=2*M_PI;
	return diff;
}

double image_add::minangulardiff(double p1, double p2) {
	double diff=angulardiff(p1,p2);
	if (fabs(diff) > M_PI) {
		if (diff > 0) diff=-(2*M_PI-diff); 
		else if (diff < 0) diff=-(2*M_PI+diff);
	}
	return diff;
}

double image_add::angulardiff(vector v1, vector v2) {
	return angulardiff(v1.angle,v2.angle);
}

double image_add::minangulardiff(vector v1, vector v2) {
	return minangulardiff(v1.angle,v2.angle);
}


double image_add::len(vector v) {
	return sqrt(v.x*v.x+v.y*v.y);
}

double image_add::angle(vector v) {
	return atan2(v.y,v.x);
}

vector image_add::norm(vector v) {
	double l=v.len;
	if (l==0) return vector(0,0);
	return v/l;
}

double image_add::getdist(vector A, vector B, vector C) {
	vector vx=B-A;
	double l=vx.len;
	if (l == 0) return 0.0;
	return ((C-A)^vx)/l;
}

double image_add::getxdist(vector A, vector B, vector C) {
	vector vx=B-A;
	double l=vx.len;
	if (l == 0) return 0.0;
	return ((C-A)*vx)/l;
}


/*
int levi_civita2(int i,int j) {
	int e=(i-j);		        
	if (e==0) return 0;
	return (e > 0)?(1):(-1);
}

int levi_civita3(int i,int j,int k) {
	int e=(i-j)*(i-k)*
		        (j-k);
	if (e==0) return 0;
	return (e > 0)?(1):(-1);
}


int levi_civita4(int i,int j,int k,int l) {
	int e=(i-j)*(i-k)*(i-l)*
		        (j-k)*(j-l)*
				      (k-l);
	if (e==0) return 0;
	return (e > 0)?(1):(-1);
}


int levi_civita5(int i,int j,int k,int l,int m) {
	int e=(i-j)*(i-k)*(i-l)*(i-m)*
		        (j-k)*(j-l)*(j-m)*
				      (k-l)*(k-m)*
					        (l-m);
	if (e==0) return 0;
	return (e > 0)?(1):(-1);
}


int levi_civita6(int i,int j,int k,int l,int m,int n) {
	int e=(i-j)*(i-k)*(i-l)*(i-m)*(i-n)*
		        (j-k)*(j-l)*(j-m)*(j-n)*
				      (k-l)*(k-m)*(k-n)*
					        (l-m)*(l-n)*
							      (m-n);
	if (e==0) return 0;
	return (e > 0)?(1):(-1);
}

matrix6 matrix6::transpose(void) {
	matrix6 m;
	for (int i=0;i<6;i++) {
		for (int j=0;j<6;j++) {
			m.set(i,j,get(j,i));
		}
	}
	return m;
}



double matrix6::subdet(int row, int col, int n) {

	double l=0;
	double s=1;
	if ((row+col)%2==1) s=-1;

	if (n == 1) return get(row, col);

	// arbitrary
	for (int i=0;i<n;i++) {

		l+=s*get(row,(col+i)%6)*subdet((row+1)%6,(col+i+1)%6,n-1);
		s=-s;
	}
	return l;
}

double matrix6::det(void) {
	double l=0;
	for (int i1=1;i1<7;i1++) 
	for (int i2=1;i2<7;i2++) 
	for (int i3=1;i3<7;i3++) 
	for (int i4=1;i4<7;i4++) 
	for (int i5=1;i5<7;i5++) 
	for (int i6=1;i6<7;i6++) {
		l+=(double) levi_civita6(i1,i2,i3,i4,i5,i6) * 
			get(0,i1-1)*get(1,i2-1)*get(2,i3-1)*get(3,i4-1)*get(4,i5-1)*get(5,i6-1);
	}
	return l;
//	return subdet(0,0,6);
}

double matrix6::cofactor(int row, int col) {
	double d=subdet((row+1)%6,(col+1)%6,5);
	if ((col+row)%2 == 1) 
		return -d;
	else
		return d;
}

matrix6 matrix6::inv(void) {
	matrix6 m;
	double d=det();
	if (d != 0) {
		for (int col=0;col<6;col++) {
			for (int row=0;row<6;row++) {
				m.set(row,col,cofactor(col,row));
			}
		}
		m/=d;
	}
	return m;
}*/

int levi_civita(array<int>^ idx) {
	int e=1;
	for (int i=0;i<idx->Length-1;i++)
		for (int j=i+1;j<idx->Length;j++)
			e*=(idx[j]-idx[i]);
	if (e==0) return 0;
	return (e > 0)?(1):(-1);
}


double matrixN::det(void) {
	array<int> ^idx=gcnew array<int>(len);
	for (int i=0;i<len;i++) idx[i]=1;
	int finished=0;
	double l=0;
	while (!finished) {
		double p=levi_civita(idx);
		if (p != 0) {
			for (int i=0;i<len;i++)
				p*=v[idx[i]-1]->v[i];
			l+=p;
		}
		int n=0;
		while (n < len) {
			if (idx[n]!=len) { 
				idx[n]++; break; 
			} 
			idx[n]=1;
			n++;
		}
		if (n >= len) finished=1;
	}
	return l;
}

double matrixN::cofactor(int r,int c) {
	matrixN m=submatrix(r,c);
	if ((r+c)%2 == 1) return -m.det(); else return m.det();
}

matrixN matrixN::inv(void) {
	matrixN m;
	m.len=len;
	double d=det();
	if (d != 0) {
		for (int c=0;c<len;c++) {
			for (int r=0;r<len;r++) {
				m[r,c]=cofactor(c,r);
			}
		}
		m/=d;
	}
	return m;
}



// EOF
