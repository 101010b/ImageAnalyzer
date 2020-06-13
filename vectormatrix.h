#pragma once

#include <math.h>

#ifndef M_PI
	#define M_PI       3.14159265358979323846
#endif

namespace image_add {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Drawing::Imaging;

#define HICKUP throw gcnew ArgumentOutOfRangeException()

	public ref class vector {
	public:
		double x,y;
		vector(void) { x=y=0; }
		vector(double ix, double iy) { x=ix;y=iy; }
		vector(const vector % v) { x=v.x;y=v.y; }
		vector operator+(vector v) { return vector(x+v.x,y+v.y); }
		vector operator+=(vector v) { x+=v.x;y+=v.y; return vector(x,y); }
		vector operator-(vector v) { return vector(x-v.x,y-v.y); }
		vector operator-=(vector v) { x-=v.x;y-=v.y; return vector(x,y); }
		double operator*(vector v) { return v.x*x + v.y*y; }
		double operator^(vector v) { return x*v.y-y*v.x; }
		vector operator*(double l) { return vector(x*l,y*l); }
		vector operator*=(double l) { x*=l;y*=l; return vector(x,y); }
		vector operator/(double l) { return vector(x/l,y/l); }
		vector operator/=(double l) { x/=l;y/=l; return vector(x,y); }
		vector operator=(vector v) { x=v.x;y=v.y; return vector(v); };
		bool operator==(vector v) { return ((v.x==x)&&(v.y==y)); }
		bool operator!=(vector v) { return ((v.x!=x)||(v.y!=y)); }
		void norm(void) { double l = getlen(); if (l > 0) { x/=l; y/=l; } }
		double getlen() { return sqrt(x*x + y*y); };
		void setlen(double n) { double l = getlen(); if (l > 0) { x*=n/l;y*=n/l; }}
		property double len {
			double get() { return getlen(); }
			void set(double n) { setlen(n); }
		};
		property double angle {
			double get() { return atan2(y,x); }
			void set(double phi) { double l=getlen();x=cos(phi)*l;y=sin(phi)*l; }
		};
		vector rotate(double s) { return vector(cos(s)*x-sin(s)*y,sin(s)*x+cos(s)*y); }
		property double default[int] {
			double get(int i) { switch (i) { case 0: return x; case 1: return y; } HICKUP; };
			void set(int i, double d) { switch (i) { case 0: x=d; return; case 1: y=d; return; } HICKUP; };
		}
		int on_line(vector v1, vector v2, double tol);
	};

	vector operator*(double l, vector v); // { return vector(v.x*l,v.y*l); }
	vector operator-(vector v); //  { return vector(-v.x,-v.y); }

	public ref class matrix {
	public:
		double xx, yx; // |xx yx|
		double xy, yy; // |xy yy|
		matrix(void) { xx=xy=yx=yy=0; }
		matrix(double vxx, double vyx, double vxy,double vyy) {xx=vxx; yx=vyx; xy=vxy; yy=vyy; }
		matrix(vector v1, vector v2) { xx=v1.x;xy=v1.y;yx=v2.x;yy=v2.y; }
		matrix(const matrix % m) { xx=m.xx;xy=m.xy;yx=m.yx;yy=m.yy; }
		matrix operator*(matrix M) { return matrix(xx*M.xx+yx*M.xy,xx*M.yx+yx*M.yy,xy*M.xx+yy*M.xy,xy*M.yx+yy*M.yy); }
		matrix operator*(double l) { return matrix(xx*l,yx*l,xy*l,yy*l); }
		matrix operator/(double l) { return matrix(xx/l,yx/l,xy/l,yy/l); }
		vector operator*(vector V) { return vector(xx*V.x+yx*V.y,xy*V.x+yy*V.y); }
		matrix operator=(matrix m) { xx=m.xx;xy=m.xy;yx=m.yx;yy=m.yy; return matrix(m); }
		double det() { return xx*yy-yx*xy; };
		matrix inv() { return matrix(yy,-yx,-xy,xx)/det(); }
		property double default[int,int] {
			double get(int row, int col) { 
				if ((row==0)&&(col==0)) return xx;if ((row==0)&&(col==1)) return yx;
				if ((row==1)&&(col==0)) return xy;if ((row==1)&&(col==1)) return yy; 
				HICKUP; }
			void set(int row, int col, double l) { 
				if ((row < 0) || (row > 1) || (col < 0) || (col > 1)) HICKUP;
				if ((row==0)&&(col==0)) xx=l;if ((row==0)&&(col==1)) yx=l;
				if ((row==1)&&(col==0)) xy=l;if ((row==1)&&(col==1)) yy=l; }
		}
	};

	public ref class vectorN {
	public:
		array<double> ^v;
		vectorN(void) { v=gcnew array<double>(0); }
		vectorN(int n) { v=gcnew array<double>(n); }
		vectorN(const vectorN % t) { v=gcnew array<double>(t.v->Length); for (int i=0;i<v->Length;i++) v[i]=t.v[i]; }
		~vectorN(void) { delete v; }
		property int len {
			int get(void) { return v->Length; }
			void set(int n) { array<double> ^a=v;v=gcnew array<double>(n); for (int i=0;i<n;i++) { if (i<a->Length) v[i]=a[i]; else v[i]=0.0; } }
		}
		property double default[int] {
			double get(int i) { if ((i < 0) || (i >= v->Length)) HICKUP; return v[i]; }
			void set(int i, double l) { if ((i < 0) || (i >= v->Length)) HICKUP; v[i]=l; }
		}
		vectorN(array<double> ^t) { v=gcnew array<double>(t->Length);for (int i=0;i<t->Length;i++) v[i]=t[i]; }

		vectorN operator+(vectorN t) { if (len != t.len) HICKUP; vectorN s;s.len=len;for (int i=0;i<len;i++) s.v[i]=v[i]+t[i]; return s; }
		vectorN operator+=(vectorN t) { if (len != t.len) HICKUP; for (int i=0;i<len;i++) v[i]+=t[i]; return vectorN(v); }

		vectorN operator-(vectorN t) { if (len != t.len) HICKUP; vectorN s;s.len=len;for (int i=0;i<len;i++) s.v[i]=v[i]-t[i]; return s; }
		vectorN operator-=(vectorN t) { if (len != t.len) HICKUP; for (int i=0;i<len;i++) v[i]-=t[i]; return vectorN(v); }
		
		double operator*(vectorN t) { if (len != t.len) HICKUP; double l=0;for (int i=0;i<len;i++) l+=v[i]*t[i]; return l; }
		vectorN operator*(double l) { vectorN s;s.len=len;for (int i=0;i<len;i++) s.v[i]=v[i]*l; return s; }
		vectorN operator*=(double l) { for (int i=0;i<len;i++) v[i]*=l; return vectorN(v); }
		
		vectorN operator/(double l) { vectorN s;s.len=len;for (int i=0;i<len;i++) s.v[i]=v[i]/l; return s; }
		vectorN operator/=(double l) { for (int i=0;i<len;i++) v[i]/=l; return vectorN(v); }

		vectorN operator=(vectorN vin) { for (int i=0;i<len;i++) v[i]=vin[i]; return vectorN(vin); };
	};

	public ref class matrixN {
	public:
		array<vectorN^> ^v;
		matrixN(void) { v=gcnew array<vectorN^>(0); }
		matrixN(int n) { v=gcnew array<vectorN^>(n); for (int i=0;i<n;i++) v[i]=gcnew vectorN(n); }
		matrixN(array<vectorN^> ^a) { v=gcnew array<vectorN^>(a->Length);for (int i=0;i<a->Length;i++) v[i]=gcnew vectorN(a[i]->v); }
		matrixN(const matrixN % t) { v=gcnew array<vectorN^>(t.v->Length); for (int i=0;i<v->Length;i++) v[i]=gcnew vectorN(t.v[i]->v); }
		~matrixN(void) { for (int i=0;i<len; i++) delete v[i]; delete v; }
		property double default[int,int] {
			double get(int row, int col) { 
				if ((col < 0) || (col >= v->Length) || (row < 0) || (row >= v->Length)) HICKUP;
				return v[col]->v[row]; }
			void set(int row, int col, double l) { 
				if ((col < 0) || (col >= v->Length) || (row < 0) || (row >= v->Length)) HICKUP;
				v[col]->v[row]=l; }
		}
		property int len {
			int get(void) { return v->Length; }
			void set(int n) { 
				array<vectorN^>^ a=v;
				v=gcnew array<vectorN^>(n); 
				for (int i=0;i<n;i++) 
					v[i]=gcnew vectorN(n);
				for (int i=0;i<n;i++) for (int j=0;j<n;j++) {
					if ((i<a->Length) && (j<a->Length)) v[i]->v[j]=a[i]->v[j]; else v[i]->v[j]=0.0; 
				} 
			}
		}
		vectorN col(int i) { if ((i < 0) || (i >= v->Length)) HICKUP; return vectorN(v[i]->v); }
		vectorN row(int r) { if ((r < 0) || (r >= v->Length)) HICKUP; vectorN t;t.len=len; for (int i=0;i<len;i++) t[i]=v[i]->v[r]; return t;  }
		matrixN submatrix(int row, int col) {
			matrixN S;
			S.len=len-1;
			for (int r=0;r<len;r++) for (int c=0;c<len;c++) {
				if ((r!=row) && (c != col)) {
					int mr=r;
					int mc=c;
					if (mr > row) mr--;
					if (mc > col) mc--;
					S[mr,mc]=v[c]->v[r];
				}
			}
			return S;
		}
		vectorN operator*(vectorN vin) { if (vin.len != v->Length) HICKUP; vectorN s;s.len=len; for (int i=0;i<len;i++) s.v[i]=row(i)*vin; return s; }
		matrixN operator*(matrixN m) { if (m.len != v->Length) HICKUP; matrixN A;A.len=len; for (int i=0;i<len;i++) for (int j=0;j<len;j++) A[i,j]=m.col(i)*row(j);  return A; }
		matrixN operator=(matrixN m) { for (int i=0;i<len;i++) v[i]=m.v[i]; return matrixN(m); }
		matrixN operator*(double l) { matrixN A;A.len=len; for (int i=0;i<len;i++) for (int j=0;j<len;j++) A.v[i]->v[j]=v[i]->v[j]*l; return A; }
		matrixN operator*=(double l) { for (int i=0;i<len;i++) for (int j=0;j<len;j++) v[i]->v[j]*=l; return matrixN(v); }
		matrixN operator/(double l) { matrixN A;A.len=len; for (int i=0;i<len;i++) for (int j=0;j<len;j++) A.v[i]->v[j]=v[i]->v[j]/l; return A; }
		matrixN operator/=(double l) { for (int i=0;i<len;i++) for (int j=0;j<len;j++) v[i]->v[j]/=l; return matrixN(v); }
		matrixN transpose(void) { matrixN m;m.len=len;for (int i=0;i<len;i++) for (int j=0;j<len;j++) m.v[i]->v[j]=v[j]->v[i]; return m; }
		double det(void);
		double cofactor(int r, int c);
		matrixN inv(void);
	};

/*
	public ref class vector6 {
	public:
		double v1,v2,v3,v4,v5,v6;
		vector6(void) { v1=v2=v3=v4=v5=v6=0.0F; }
		vector6(double a1, double a2, double a3, double a4, double a5, double a6) { v1=a1;v2=a2;v3=a3;v4=a4;v5=a5;v6=a6; }
		vector6(const vector6 % v) { v1=v.v1;v2=v.v2;v3=v.v3;v4=v.v4;v5=v.v5;v6=v.v6; }
		vector6 operator+(vector6 v) { return vector6(v1+v.v1,v2+v.v2,v3+v.v3,v4+v.v4,v5+v.v5,v6+v.v6); }
		vector6 operator+=(vector6 v) { v1+=v.v1;v2+=v.v2;v3+=v.v3;v4+=v.v4;v5+=v.v5;v6+=v.v6; return vector6(v1,v2,v3,v4,v5,v6); }
		vector6 operator-(vector6 v) { return vector6(v1-v.v1,v2-v.v2,v3-v.v3,v4-v.v4,v5-v.v5,v6-v.v6); }
		vector6 operator-=(vector6 v) { v1-=v.v1;v2-=v.v2;v3-=v.v3;v4-=v.v4;v5-=v.v5;v6-=v.v6; return vector6(v1,v2,v3,v4,v5,v6); }
		double operator*(vector6 v) { return v1*v.v1+v2*v.v2+v3*v.v3+v4*v.v4+v5*v.v5+v6*v.v6; }
		vector6 operator*(double l) { return vector6(v1*l,v2*l,v3*l,v4*l,v5*l,v6*l); }
		vector6 operator*=(double l) { v1*=l;v2*=l;v3*=l;v4*=l;v5*=l;v6*=l; return vector6(v1,v2,v3,v4,v5,v6); }
		vector6 operator/(double l) { return vector6(v1/l,v2/l,v3/l,v4/l,v5/l,v6/l); }
		vector6 operator/=(double l) { v1/=l;v2/=l;v3/=l;v4/=l;v5/=l;v6/=l; return vector6(v1,v2,v3,v4,v5,v6); }
		vector6 operator=(vector6 v) { v1=v.v1;v2=v.v2;v3=v.v3;v4=v.v4;v5=v.v5;v6=v.v6; return vector6(v); };
		property double default[int] {
			double get(int i) { switch(i) { case 0: return v1;case 1: return v2;case 2: return v3;case 3: return v4;case 4: return v5;case 5: return v6; } HICKUP; }
			void set(int i, double l) { switch(i) { case 0: v1=l;return;case 1: v2=l;return;case 2: v3=l;return;case 3: v4=l;return;case 4: v5=l;return;case 5: v6=l;return; } }
		}
		//double operator[](int i) { switch(i) { case 0: return v1;case 1: return v2;case 2: return v3;case 3: return v4;case 4: return v5;case 5: return v6; } }
		void norm(void) { double l = getlen(); if (l > 0) { v1/=l;v2/=l;v3/=l;v4/=l;v5/=l;v6/=l; } }
		double getlen() { return sqrt(v1*v1 + v2*v2 +v3*v3 +v4*v4 +v5*v5 +v6*v6); }
		void setlen(double n) { double l = getlen(); if (l > 0) { v1*=n/l;v2*=n/l;v3*=n/l;v4*=n/l;v5*=n/l;v6*=n/l; } }
		property double len {
			double get() { return getlen(); }
			void set(double n) { setlen(n); }
		};
	};

	public ref class matrix6 {
	public:
		vector6 v1,v2,v3,v4,v5,v6;
		matrix6(void) { v1=v2=v3=v4=v5=v6=vector6(0,0,0,0,0,0); }
		matrix6(vector6 a1,vector6 a2,vector6 a3,vector6 a4,vector6 a5,vector6 a6) { v1=a1;v2=a2;v3=a3;v4=a4;v5=a5;v6=a6; }
		matrix6(const matrix6 % m) { v1=m.v1;v2=m.v2;v3=m.v3;v4=m.v4;v5=m.v5;v6=m.v6; }
		vector6 col(int i) { switch (i) { case 0: return v1;case 1: return v2;case 2: return v3;case 3: return v4;case 4: return v5;case 5: return v6; } HICKUP; }
		vector6 row(int i) { return vector6(v1[i],v2[i],v2[i],v2[i],v2[i],v2[i]); }
		double _get(int row, int col ) { switch(col) { case 0: return v1[row]; case 1: return v2[row]; case 2: return v3[row]; case 3: return v4[row]; case 4: return v5[row]; case 5: return v6[row]; } HICKUP }
		void _set(int row, int col, double l) { switch(col) { case 0: v1[row]=l;return; case 1: v2[row]=l;return; case 2: v3[row]=l;return; case 3: v4[row]=l;return; case 4: v5[row]=l;return; case 5: v6[row]=l;return; } }
		double get(int row, int col ) { return _get(row,col); }
		void set(int row, int col, double l) { _set(row,col,l); }
		vector6 operator*(vector6 v) { return vector6(row(1)*v,row(2)*v,row(3)*v,row(4)*v,row(5)*v,row(6)*v); }
		matrix6 operator*(matrix6 m) { matrix6 A; for (int i=0;i<6;i++) { for (int j=0;j<6;j++) A.set(i,j,row(j)*m.col(i)); } return A; }
		matrix6 operator=(matrix6 m) { v1=m.v1;v2=m.v2;v3=m.v3;v4=m.v4;v5=m.v5;v6=m.v6; return matrix6(m); }
		matrix6 operator*(double l) { return matrix6(v1*l,v2*l,v3*l,v4*l,v5*l,v6*l); }
		matrix6 operator/(double l) { return matrix6(v1/l,v2/l,v3/l,v4/l,v5/l,v6/l); }
		matrix6 operator*=(double l) { v1*=l;v2*=l;v3*=l;v4*=l;v5*=l;v6*=l; return matrix6(v1,v2,v3,v4,v5,v6); }
		matrix6 operator/=(double l) { v1/=l;v2/=l;v3/=l;v4/=l;v5/=l;v6/=l; return matrix6(v1,v2,v3,v4,v5,v6); }
		matrix6 transpose(void);
		double subdet(int row, int col, int n);
		double det(void);
		double cofactor(int row, int col);
		matrix6 inv(void);
		property double default[int,int] {
			double get(int row, int col) { return _get(row,col); }
			void set(int row, int col, double l) { _set(row,col,l); }
		}
		
	};*/

	double angulardiff(double p1, double p2);
	double minangulardiff(double p1, double p2);

	double angulardiff(vector v1, vector v2);
	double minangulardiff(vector v1, vector v2);

	double len(vector v);
	double angle(vector v);
	vector norm(vector v);
	double getdist(vector A, vector B, vector C);
	double getxdist(vector A, vector B, vector C);
}