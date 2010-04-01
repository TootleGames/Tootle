/*------------------------------------------------------
	Expanded/FORCEINLINEd code for TLTypes.h

-------------------------------------------------------*/


namespace TLMaths
{
	//	gr: forward declarations so we don't need to include
	float		Cosf(float RadAng);
	float		Sinf(float RadAng);
	float		Sqrtf(float);
}

//	forward declarations
namespace TLDebug
{
	extern Bool		CheckIndex(int Index,int Max);
};


template <typename TYPE>
class Type2
{
public:
	TYPE	x;
	TYPE	y;

public:
	Type2()								: x(0), y(0)				{	};
	explicit Type2(const TYPE& a)		: x(a), y(a)				{	};
	Type2(const TYPE& a, const TYPE& b)	: x(a), y(b)				{	};
	template<typename OTHERTYPE> Type2(const Type2<OTHERTYPE>& t) : x(t.x), y(t.y)	{	};

	FORCEINLINE static u32		GetSize() 									{	return 2;	}
	FORCEINLINE void			Set(const TYPE& a, const TYPE& b)			{	x = a;	y = b;	};
	FORCEINLINE void			Set(const Type2<TYPE>& ab)					{	x = ab.x;	y = ab.y;	};
	FORCEINLINE void			Set(const Type3<TYPE>& abc)					{	x = abc.x;	y = abc.y; };
	FORCEINLINE Type3<TYPE>		xyz(const TYPE& z) const					{	return Type3<TYPE>( x, y, z );	}
	FORCEINLINE Bool			IsZero() const								{	return (x==0) && (y==0);	}	//	could also use {	return DotProduct() == 0;	}	
	FORCEINLINE Bool			IsNonZero() const							{	return (x!=0) || (y!=0);	}	//	could also use {	return DotProduct() != 0;	}	

	FORCEINLINE Type2<TYPE>		operator *(const TYPE& v) const 		{	return Type2<TYPE>( x*v, y*v );	};
	FORCEINLINE Type2<TYPE>		operator *(const Type2<TYPE>& v) const	{	return Type2<TYPE>( x*v.x, y*v.y );	};
	FORCEINLINE Type2<TYPE>		operator +(const TYPE& v) const			{	return Type2<TYPE>( x+v, y+v );	};
	FORCEINLINE Type2<TYPE>		operator +(const Type2<TYPE>& v) const	{	return Type2<TYPE>( x+v.x, y+v.y );	};
	FORCEINLINE Type2<TYPE>		operator -(const TYPE& v) const			{	return Type2<TYPE>( x-v, y-v );	};
	FORCEINLINE Type2<TYPE>		operator -(const Type2<TYPE>& v) const	{	return Type2<TYPE>( x-v.x, y-v.y );	};
	FORCEINLINE Type2<TYPE>		operator /(const TYPE& v) const			{	return Type2<TYPE>( x/v, y/v );	};
	FORCEINLINE Type2<TYPE>		operator /(const Type2<TYPE>& v) const	{	return Type2<TYPE>( x/v.x, y/v.y );	};

	FORCEINLINE Bool			operator==(const Type2<TYPE>& v) const		{	return ( x == v.x ) && ( y == v.y );	};
	FORCEINLINE Bool			operator!=(const Type2<TYPE>& v) const		{	return ( x != v.x ) || ( y != v.y );	};
	FORCEINLINE Bool			operator<(const Type2<TYPE>& v) const		{	return ( x < v.x ) && ( y < v.y );	};

	template<typename OTHERTYPE> FORCEINLINE void		operator=(const Type2<OTHERTYPE>& v);
	template<typename OTHERTYPE> FORCEINLINE void		operator=(const Type3<OTHERTYPE>& v);
	template<typename OTHERTYPE> FORCEINLINE void		operator=(const Type4<OTHERTYPE>& v);
	
	FORCEINLINE void		operator+=(const TYPE& v)				{	x += v;		y += v;	};
	FORCEINLINE void		operator-=(const TYPE& v)				{	x -= v;		y -= v;	};
	FORCEINLINE void		operator*=(const TYPE& v)				{	x *= v;		y *= v;	};
	FORCEINLINE void		operator/=(const TYPE& v)				{	x /= v;		y /= v;	};

	FORCEINLINE void		operator+=(const Type2<TYPE>& v)		{	x += v.x;		y += v.y;	};
	FORCEINLINE void		operator-=(const Type2<TYPE>& v)		{	x -= v.x;		y -= v.y;	};
	FORCEINLINE void		operator*=(const Type2<TYPE>& v)		{	x *= v.x;		y *= v.y;	};
	FORCEINLINE void		operator/=(const Type2<TYPE>& v)		{	x /= v.x;		y /= v.y;	};

	FORCEINLINE void		operator+=(const Type3<TYPE>& v);
	FORCEINLINE void		operator-=(const Type3<TYPE>& v);
	FORCEINLINE void		operator*=(const Type3<TYPE>& v);
	FORCEINLINE void		operator/=(const Type3<TYPE>& v);

	FORCEINLINE TYPE&		operator[](const int Index)		{	TLDebug::CheckIndex(Index,2);	return GetData()[Index];	};
	FORCEINLINE const TYPE*	GetData() const					{	return &x;	};
	FORCEINLINE TYPE*		GetData()						{	return &x;	};
	FORCEINLINE				operator TYPE*()				{	return GetData();	};
	FORCEINLINE				operator const TYPE*() const	{	return GetData();	};

	FORCEINLINE TYPE&			Left()										{	return x;	}
	FORCEINLINE TYPE&			Top()						{	return y;	}
	FORCEINLINE const TYPE&		Left() const				{	return x;	}
	FORCEINLINE const TYPE&		Top() const					{	return y;	}

	//	vector functions
	FORCEINLINE TYPE			LengthSq() const				{	return (x*x) + (y*y);	};
	FORCEINLINE TYPE			Length() const					{	return TLMaths::Sqrtf( LengthSq() );	};
	
	FORCEINLINE void			Normalise(float NormalLength=1.f)				{	(*this) *= NormalLength/Length();	};			//	normalises vector
	FORCEINLINE void			Normalise(float KnownLength,float NormalLength)	{	(*this) *= NormalLength/KnownLength;	};			//	normalises vector
	FORCEINLINE Type2<TYPE>		Normal(float NormalLength=1.f) const			{	return (*this) * (NormalLength/Length());	};	//	returns the normal of thsi vector
	FORCEINLINE Type2<TYPE>		Normal(float KnownLength,float NormalLength) const		{	return (*this) * (NormalLength/KnownLength);	};	//	returns the normal of thsi vector
	FORCEINLINE void			Invert()									{	x=-x;	y=-y;	};
//	FORCEINLINE void			Reflect(const Type2<TYPE>& UpVector);
	Type2<TYPE>					CrossProduct(const Type2<TYPE>& v) const;
	float						CrossProductScalar(const Type2<TYPE>& v) const;
	FORCEINLINE float			DotProduct(const Type2<TYPE>& v) const		{	return (x*v.x) + (y*v.y) ;	}
	FORCEINLINE float			DotProduct() const							{	return (x*x) + (y*y);	}	//	same as lengthsq!

	FORCEINLINE void			RotateAntiClockwise()							{	float2 oldxy(x,y);	x = oldxy.y;	y = -oldxy.x;	}
	FORCEINLINE void			RotateClockwise()								{	float2 oldxy(x,y);	x = -oldxy.y;	y = oldxy.x;	}
	FORCEINLINE void			Rotate(float AngRad);
	FORCEINLINE Type2<TYPE>		GetAntiClockwise() const						{	return Type2<TYPE>( y, -x );	}
	FORCEINLINE Type2<TYPE>		GetClockwise() const							{	return Type2<TYPE>( -y, x );	}
};





template <typename TYPE>
class Type3
{
public:
	Type3()														: x(0), y(0), z(0)	{	}
	explicit Type3(const TYPE& a)								: x(a), y(a), z(a)	{	}
	Type3(const TYPE& a, const TYPE& b, const TYPE& c)			: x(a), y(b), z(c)	{	}
//	template<typename OTHERTYPE> Type3(const Type3<OTHERTYPE>& t)			{	Set( t.x, t.y, t.z );	};
	Type3(const Type2<TYPE>& t,TYPE tz=0.f)						: x(t.x), y(t.y), z(tz)				{	}
	Type3(const Type3<TYPE>& t)									: x(t.x), y(t.y), z(t.z)				{	}
	template<typename OTHERTYPE> Type3(const Type4<OTHERTYPE>& t);

	FORCEINLINE static u32			GetSize() 											{	return 3;	}
	FORCEINLINE void				Set(const TYPE& a, const TYPE& b, const TYPE& c)	{	x = a;	y = b;	z = c;	};
	FORCEINLINE void				Set(const Type3<TYPE>& abc)							{	x = abc.x;	y = abc.y;	z = abc.z;	};
	FORCEINLINE Type2<TYPE>&		xy()												{	return *((Type2<TYPE>*)&x);	}
	FORCEINLINE const Type2<TYPE>&	xy() const											{	return *((Type2<TYPE>*)&x);	}
	FORCEINLINE Type4<TYPE>			xyzw(const TYPE& w) const;
	FORCEINLINE Bool				IsZero() const										{	return (x==0) && (y==0) && (z==0);	}	//	could also use {	return DotProduct() == 0;	}	
	FORCEINLINE Bool				IsNonZero() const									{	return (x!=0) || (y!=0) || (z!=0);	}	//	could also use {	return DotProduct() != 0;	}	
	FORCEINLINE Bool				HasDifferenceMin(const Type3<TYPE>& Other,const TYPE& MinDifference) const;			//	check difference between two instances are above a miniumum

	FORCEINLINE TYPE		LengthSq() const									{	return (x*x) + (y*y) + (z*z);	};
	FORCEINLINE TYPE		Length() const										{	return sqrtf( LengthSq() );	};

	FORCEINLINE Bool		operator==(const Type3<TYPE>& v) const				{	return ( x == v.x ) && ( y == v.y ) && ( z == v.z );	};
	FORCEINLINE Bool		operator!=(const Type3<TYPE>& v) const				{	return ( x != v.x ) || ( y != v.y ) || ( z != v.z );	};
	FORCEINLINE Bool		operator<(const Type3<TYPE>& v) const				{	return ( x < v.x ) && ( y < v.y ) && ( z < v.z );	};
	
	template<typename OTHERTYPE> FORCEINLINE void		operator=(const Type2<OTHERTYPE>& v);
	template<typename OTHERTYPE> FORCEINLINE void		operator=(const Type3<OTHERTYPE>& v);
	template<typename OTHERTYPE> FORCEINLINE void		operator=(const Type4<OTHERTYPE>& v);

	FORCEINLINE void		operator+=(const TYPE v)				{	x += v;		y += v;		z += v;	};
	FORCEINLINE void		operator-=(const TYPE v)				{	x -= v;		y -= v;		z -= v;	};
	FORCEINLINE void		operator*=(const TYPE v)				{	x *= v;		y *= v;		z *= v;	};
	FORCEINLINE void		operator/=(const TYPE v)				{	x /= v;		y /= v;		z /= v;	};

	FORCEINLINE void		operator+=(const Type3<TYPE>& v)		{	x += v.x;		y += v.y;		z += v.z;	};
	FORCEINLINE void		operator-=(const Type3<TYPE>& v)		{	x -= v.x;		y -= v.y;		z -= v.z;	};
	FORCEINLINE void		operator*=(const Type3<TYPE>& v)		{	x *= v.x;		y *= v.y;		z *= v.z;	};
	FORCEINLINE void		operator/=(const Type3<TYPE>& v)		{	x /= v.x;		y /= v.y;		z /= v.z;	};

	FORCEINLINE void		operator+=(const Type4<TYPE>& v)		{	x += v.x;		y += v.y;		z += v.z;	};
	FORCEINLINE void		operator-=(const Type4<TYPE>& v)		{	x -= v.x;		y -= v.y;		z -= v.z;	};
	FORCEINLINE void		operator*=(const Type4<TYPE>& v)		{	x *= v.x;		y *= v.y;		z *= v.z;	};
	FORCEINLINE void		operator/=(const Type4<TYPE>& v)		{	x /= v.x;		y /= v.y;		z /= v.z;	};

	template<typename OPTYPE>
	FORCEINLINE Type3<TYPE>	operator *(const OPTYPE& v) const		{	return Type3<TYPE>( x*v, y*v, z*v );	};
	template<typename OPTYPE>
	FORCEINLINE Type3<TYPE>	operator /(const OPTYPE& v) const		{	return Type3<TYPE>( x/v, y/v, z/v );	};
	template<typename OPTYPE>
	FORCEINLINE Type3<TYPE>	operator -(const OPTYPE& v) const		{	return Type3<TYPE>( x/v, y/v, z/v );	};
	template<typename OPTYPE>
	FORCEINLINE Type3<TYPE>	operator +(const OPTYPE& v) const		{	return Type3<TYPE>( x+v, y+v, z+v );	};

	FORCEINLINE Type3<TYPE>	operator *(const Type3<TYPE>& v) const	{	return Type3<TYPE>( x*v.x, y*v.y, z*v.z );	};
	FORCEINLINE Type3<TYPE>	operator /(const Type3<TYPE>& v) const	{	return Type3<TYPE>( x/v.x, y/v.y, z/v.z );	};
	FORCEINLINE Type3<TYPE>	operator -(const Type3<TYPE>& v) const	{	return Type3<TYPE>( x-v.x, y-v.y, z-v.z );	};
	FORCEINLINE Type3<TYPE>	operator +(const Type3<TYPE>& v) const	{	return Type3<TYPE>( x+v.x, y+v.y, z+v.z );	};

	FORCEINLINE Type3<TYPE>	operator *(const Type4<TYPE>& v) const	{	return Type3<TYPE>( x*v.x, y*v.y, z*v.z );	};
	FORCEINLINE Type3<TYPE>	operator /(const Type4<TYPE>& v) const	{	return Type3<TYPE>( x/v.x, y/v.y, z/v.z );	};
	FORCEINLINE Type3<TYPE>	operator -(const Type4<TYPE>& v) const	{	return Type3<TYPE>( x-v.x, y-v.y, z-v.z );	};
	FORCEINLINE Type3<TYPE>	operator +(const Type4<TYPE>& v) const	{	return Type3<TYPE>( x+v.x, y+v.y, z+v.z );	};

	FORCEINLINE TYPE&			operator[](const int Index)				{	TLDebug::CheckIndex(Index,3);	return GetData()[Index];	};
	FORCEINLINE const TYPE*		GetData() const							{	return &x;	};
	FORCEINLINE TYPE*			GetData()								{	return &x;	};
	FORCEINLINE					operator TYPE*()						{	return GetData();	};
	FORCEINLINE					operator const TYPE*() const			{	return GetData();	};

	//	vector functions
	FORCEINLINE void			Normalise(float NormalLength=1.f)				{	(*this) *= NormalLength/Length();	};			//	normalises vector
	FORCEINLINE void			Normalise(float KnownLength,float NormalLength)	{	(*this) *= NormalLength/KnownLength;	};			//	normalises vector
	FORCEINLINE Type3<TYPE>		Normal(float NormalLength=1.f) const			{	return (*this) * (NormalLength/Length());	};	//	returns the normal of thsi vector
	FORCEINLINE Type3<TYPE>		Normal(float KnownLength,float NormalLength) const		{	return (*this) * (NormalLength/KnownLength);	};	//	returns the normal of thsi vector
	FORCEINLINE void			Invert()									{	x=-x;	y=-y;	z=-z;	};
	FORCEINLINE void			Reflect(const Type3<TYPE>& UpVector);
	Type3<TYPE>					CrossProduct(const Type3<TYPE>& v) const;
	FORCEINLINE float			DotProduct(const Type3<TYPE>& v) const		{	return (x*v.x) + (y*v.y) + (z*v.z);	}
	FORCEINLINE float			DotProduct() const							{	return (x*x) + (y*y) + (z*z);	}	//	same as lengthsq!
	//Bool						InsideTriangle(Type3<TYPE>& v0, Type3<TYPE>& v1, Type3<TYPE>& v2, GRPlaneEq& Plane);
	void						RotateX(float RadAng);
	void						RotateY(float RadAng);
	void						RotateZ(float RadAng);
	void						SetRotation( float Angle, float Elevation, float Length );
	void						SortComponents();

public:
	TYPE	x;
	TYPE	y;
	TYPE	z;
};




template <typename TYPE>
class Type4
{
public:
	Type4()																: x(0), y(0), z(0), w(0)				{}
	explicit Type4(const TYPE& a)										: x(a), y(a), z(a), w(a)				{}
	Type4(const TYPE& a, const TYPE& b, const TYPE& c, const TYPE& d)	: x(a), y(b), z(c), w(d)				{}
	Type4(TYPE* p)														: x(p[0]), y(p[1]), z(p[2]), w(p[3])	{}
	template<typename OTHERTYPE> Type4(const Type4<OTHERTYPE>& t)	{	Set( (TYPE)t.x, (TYPE)t.y, (TYPE)t.z, (TYPE)t.w );	};	
	template<typename OTHERTYPE> Type4(const Type3<OTHERTYPE>& t)	{	Set( (TYPE)t.x, (TYPE)t.y, (TYPE)t.z, (TYPE)0 );	};

	FORCEINLINE static u32			GetSize() 									{	return 4;	}
	FORCEINLINE void				Set(const TYPE& a, const TYPE& b, const TYPE& c, const TYPE& d)	{	x = a;	y = b;	z = c;	w = d;	};
	FORCEINLINE void				Set(const Type3<TYPE>& v, const TYPE d)						{	Set( v.x, v.y, v.z, d );	};
	FORCEINLINE void				Set(const Type4<TYPE>& v)									{	Set( v.x, v.y, v.z, v.w );	};
	FORCEINLINE Type2<TYPE>&		xy()									{	return *((Type2<TYPE>*)&x);	}
	FORCEINLINE const Type2<TYPE>&	xy() const								{	return *((Type2<TYPE>*)&x);	}
	FORCEINLINE Type3<TYPE>&		xyz()									{	return *((Type3<TYPE>*)&x);	}
	FORCEINLINE const Type3<TYPE>&	xyz() const								{	return *((Type3<TYPE>*)&x);	}
	FORCEINLINE Type2<TYPE>&		zw()									{	return *((Type2<TYPE>*)&z);	}
	FORCEINLINE const Type2<TYPE>&	zw() const								{	return *((Type2<TYPE>*)&z);	}
	FORCEINLINE Bool				IsZero() const							{	return (x==0) && (y==0) && (z==0) && (w==0);	}	//	could also use {	return DotProduct() == 0;	}	
	FORCEINLINE Bool				IsNonZero() const						{	return (x!=0) || (y!=0) || (z!=0) || (w!=0);	}	//	could also use {	return DotProduct() != 0;	}	
	FORCEINLINE Bool				HasDifferenceMin(const Type4<TYPE>& Other,const TYPE& MinDifference) const;			//	check difference between two instances are above a miniumum

	FORCEINLINE Bool			operator==(const Type4<TYPE>& v) const		{	return ( x == v.x ) && ( y == v.y ) && ( z == v.z ) && ( w == v.w );	};
	FORCEINLINE Bool			operator!=(const Type4<TYPE>& v) const		{	return ( x != v.x ) || ( y != v.y ) || ( z != v.z ) || ( w != v.w );	};
	FORCEINLINE Bool			operator<(const Type4<TYPE>& v) const		{	return ( x < v.x ) && ( y < v.y ) && ( z < v.z ) && ( w < v.w );	};

	template<typename OTHERTYPE> FORCEINLINE void		operator=(const Type2<OTHERTYPE>& v);
	template<typename OTHERTYPE> FORCEINLINE void		operator=(const Type3<OTHERTYPE>& v);
	template<typename OTHERTYPE> FORCEINLINE void		operator=(const Type4<OTHERTYPE>& v);

	FORCEINLINE void			operator+=(const Type4<TYPE>& v)		{	x += v.x;		y += v.y;		z += v.z;		w += v.w;	};
	FORCEINLINE void			operator-=(const Type4<TYPE>& v)		{	x -= v.x;		y -= v.y;		z -= v.z;		w -= v.w;	};
	FORCEINLINE void			operator*=(const Type4<TYPE>& v)		{	x *= v.x;		y *= v.y;		z *= v.z;		w *= v.w;	};
	FORCEINLINE void			operator/=(const Type4<TYPE>& v)		{	x /= v.x;		y /= v.y;		z /= v.z;		w /= v.w;	};

	FORCEINLINE void			operator+=(const TYPE v)				{	x += v;		y += v;		z += v;		w += v;	};
	FORCEINLINE void			operator-=(const TYPE v)				{	x -= v;		y -= v;		z -= v;		w -= v;	};
	FORCEINLINE void			operator*=(const TYPE v)				{	x *= v;		y *= v;		z *= v;		w *= v;	};
	FORCEINLINE void			operator/=(const TYPE v)				{	x /= v;		y /= v;		z /= v;		w /= v;	};
	
	FORCEINLINE Type4<TYPE>	operator+(const Type4<TYPE>& v) const	{	return Type4<TYPE>( x + v.x,	y + v.y,	z + v.z,	w + v.w );	};
	FORCEINLINE Type4<TYPE>	operator-(const Type4<TYPE>& v) const	{	return Type4<TYPE>( x - v.x,	y - v.y,	z - v.z,	w - v.w );	};
	FORCEINLINE Type4<TYPE>	operator*(const Type4<TYPE>& v) const	{	return Type4<TYPE>( x * v.x,	y * v.y,	z * v.z,	w * v.w );	};
	FORCEINLINE Type4<TYPE>	operator/(const Type4<TYPE>& v) const	{	return Type4<TYPE>( x / v.x,	y / v.y,	z / v.z,	w / v.w );	};

	FORCEINLINE Type4<TYPE>	operator+(const TYPE v) const			{	return Type4<TYPE>( x + v,	y + v,	z + v,	w + v );	};
	FORCEINLINE Type4<TYPE>	operator-(const TYPE v) const			{	return Type4<TYPE>( x - v,	y - v,	z - v,	w - v );	};
	FORCEINLINE Type4<TYPE>	operator*(const TYPE v) const			{	return Type4<TYPE>( x * v,	y * v,	z * v,	w * v );	};
	FORCEINLINE Type4<TYPE>	operator/(const TYPE v) const			{	return Type4<TYPE>( x / v,	y / v,	z / v,	w / v );	};

	FORCEINLINE TYPE&		operator[](const int Index)		{	TLDebug::CheckIndex(Index,4);	return GetData()[Index];	};
	FORCEINLINE const TYPE*	GetData() const					{	return &x;	};
	FORCEINLINE TYPE*		GetData()						{	return &x;	};
	FORCEINLINE				operator TYPE*()				{	return GetData();	};
	FORCEINLINE				operator const TYPE*() const	{	return GetData();	};

	//	for use for rect's - just to clarify this.w doesnt really mean width...
	FORCEINLINE TYPE&		Left()										{	return x;	}
	FORCEINLINE TYPE&		Top()						{	return y;	}
	FORCEINLINE const TYPE&	Left() const				{	return x;	}
	FORCEINLINE const TYPE&	Top() const					{	return y;	}
	FORCEINLINE TYPE&		Width()						{	return z;	}
	FORCEINLINE TYPE&		Height()					{	return w;	}
	FORCEINLINE const TYPE&	Width() const				{	return z;	}
	FORCEINLINE const TYPE&	Height() const				{	return w;	}
	FORCEINLINE TYPE		HalfWidth() const			{	return Width() * 0.5f;	}
	FORCEINLINE TYPE		HalfHeight() const			{	return Height() * 0.5f;	}
	FORCEINLINE TYPE		Right() const				{	return x + z;	}
	FORCEINLINE TYPE		Bottom() const				{	return y + w;	}
	FORCEINLINE Type2<TYPE>	GetCenter() const			{	return Type2<TYPE>( Left() + HalfWidth(), Top() + HalfHeight() );	}
	FORCEINLINE Bool		GetIsInside(const Type2<TYPE>& Pos) const	{	return (Pos.x >= Left()) && (Pos.x <= Right()) && (Pos.y >= Top()) && (Pos.y <= Bottom());	}

	//	vector functions
	FORCEINLINE void		Invert()									{	x=-x;	y=-y;	z=-z;	w=-w;	};	
	FORCEINLINE void		Normalise(float NormalLength=1.f)			{	(*this) *= NormalLength/Length();	};			//	normalises vector
	Type4<TYPE>				Normal(float NormalLength=1.f) const		{	return (*this) * (NormalLength/Length());	};	//	returns the normal of thsi vector
	FORCEINLINE TYPE		LengthSq() const							{	return (x*x) + (y*y) + (z*z) + (w*w);	};
	FORCEINLINE TYPE		Length() const								{	return sqrtf( LengthSq() );	};
	float					DotProduct(const Type4<TYPE>& v) const		{	return (x * v.x) + (y * v.y) + (z * v.z) + (w * v.w);	}

	//	2d functions
	FORCEINLINE Bool		PointInside(const Type2<TYPE>& Point) const	{	return ( Point.x >= x && Point.y >= y && Point.x < z && Point.y < w );	};

public:
	TYPE	x;
	TYPE	y;
	TYPE	z;
	TYPE	w;

};





template <typename TYPE>
FORCEINLINE void Type2<TYPE>::operator+=(const Type3<TYPE>& v)		{	x += v.x;		y += v.y;	};

template <typename TYPE>
FORCEINLINE void Type2<TYPE>::operator-=(const Type3<TYPE>& v)		{	x -= v.x;		y -= v.y;	};

template <typename TYPE>
FORCEINLINE void Type2<TYPE>::operator*=(const Type3<TYPE>& v)		{	x *= v.x;		y *= v.y;	};

template <typename TYPE>
FORCEINLINE void Type2<TYPE>::operator/=(const Type3<TYPE>& v)		{	x /= v.x;		y /= v.y;	};



template <typename TYPE>
Type2<TYPE> Type2<TYPE>::CrossProduct(const Type2<TYPE>& v) const
{
	Type2<TYPE> xyz;

	xyz.x = ( x * v.y ) - ( y * v.x );
	xyz.y = ( y * v.x ) - ( x * v.y );

	return xyz;
}

//------------------------------------------------------
//	gr: from box2d
//------------------------------------------------------
template <typename TYPE>
float Type2<TYPE>::CrossProductScalar(const Type2<TYPE>& v) const
{
	return x * v.y - y * v.x;
}


template <typename TYPE>
FORCEINLINE void Type2<TYPE>::Rotate(float AngleRad)
{
	float2 Old(x,y);

	x = (TLMaths::Cosf(AngleRad)*Old.x) - (TLMaths::Sinf(AngleRad)*Old.y);
	y = (TLMaths::Sinf(AngleRad)*Old.x) + (TLMaths::Cosf(AngleRad)*Old.y);
}


template <typename TYPE>
template<typename OTHERTYPE> 
FORCEINLINE Type3<TYPE>::Type3(const Type4<OTHERTYPE>& t)
{
	Set( t.x, t.y, t.z );
}



template <typename TYPE>
FORCEINLINE Type4<TYPE> Type3<TYPE>::xyzw(const TYPE& w) const
{
	return Type4<TYPE>( x, y, z, w );
}



template <typename TYPE>
Type3<TYPE> Type3<TYPE>::CrossProduct(const Type3<TYPE>& v) const
{
	Type3<TYPE> xyz;

	xyz.x = ( y * v.z ) - ( z * v.y );
	xyz.y = ( z * v.x ) - ( x * v.z );
	xyz.z = ( x * v.y ) - ( y * v.x );

	return xyz;
}


template <class TYPE>
FORCEINLINE void Type3<TYPE>::Reflect(const Type3<TYPE>& UpVector)
{
	float Prod = UpVector.DotProduct( *this );
	(*this) -= ( UpVector * ( 2.f * Prod ) );
}



template <class TYPE>
void Type3<TYPE>::RotateX(float RadAng)
{
	float y0 = y;	float z0 = z;
	y = ( y0 * TLMaths::Cosf( RadAng ) ) - ( z0 * TLMaths::Sinf( RadAng ) );  
	z = ( y0 * TLMaths::Sinf( RadAng ) ) + ( z0 * TLMaths::Cosf( RadAng ) );
};



template <class TYPE>
void Type3<TYPE>::RotateY(float RadAng)
{
	float x0 = x;	float z0 = z;
	x = ( x0 * TLMaths::Cosf( RadAng ) ) + ( z0 * TLMaths::Sinf( RadAng ) );
	z = ( z0 * TLMaths::Cosf( RadAng ) ) - ( x0 * TLMaths::Sinf( RadAng ) );
};



template <class TYPE>
void Type3<TYPE>::RotateZ(float RadAng)
{
	float x0 = x;	float y0 = y;
	x = ( x0 * TLMaths::Cosf( RadAng ) ) - ( y0 * TLMaths::Sinf( RadAng ) );  
	y = ( y0 * TLMaths::Cosf( RadAng ) ) + ( x0 * TLMaths::Sinf( RadAng ) );
};


template <class TYPE>
void Type3<TYPE>::SetRotation( float Angle, float Elevation, float Length )
{
	//	sets a position from rotation
	y = TLMaths::Cosf( Elevation ) * Length;
	x = TLMaths::Sinf( Angle ) * Length;
	z = TLMaths::Cosf( Angle ) * Length;
}


template <class TYPE>
void Type3<TYPE>::SortComponents()
{
	//	sort x/y
	if ( y < x )
	{
		TYPE tmp = x;
		x = y;
		y = tmp;
	}

	if ( z < x )
	{
		TYPE tmp = x;
		x = z;
		z = tmp;
	}
	else if ( z < y )
	{
		TYPE tmp = y;
		y = z;
		z = tmp;
	}
}


template <class TYPE>
FORCEINLINE Bool Type3<TYPE>::HasDifferenceMin(const Type3<TYPE>& Other,const TYPE& MinDifference) const
{
	//	gr: will give wrong results if min difference is negative!
	//	gr: lengthsq check here might be better? or a dot product?
	/*
	float OldLengthSq = m_Translate.LengthSq();
	float NewLengthSq = Translate.LengthSq();
	if ( (OldLengthSq - NewLengthSq) < -MinChange ||
		 (OldLengthSq - NewLengthSq) > MinChange )
	*/
	return ( (Other.x - x) < -MinDifference ||
			 (Other.x - x) > MinDifference ||
			 (Other.y - y) < -MinDifference ||
			 (Other.y - y) > MinDifference ||
			 (Other.z - z) < -MinDifference ||
			 (Other.z - z) > MinDifference );
}


template <class TYPE>
FORCEINLINE Bool Type4<TYPE>::HasDifferenceMin(const Type4<TYPE>& Other,const TYPE& MinDifference) const
{
	//	gr: will give wrong results if min difference is negative!
	//	gr: lengthsq check here might be better? or a dot product?
	/*
	float OldLengthSq = m_Translate.LengthSq();
	float NewLengthSq = Translate.LengthSq();
	if ( (OldLengthSq - NewLengthSq) < -MinChange ||
		 (OldLengthSq - NewLengthSq) > MinChange )
	*/
	return ( (Other.x - x) < -MinDifference ||
			 (Other.x - x) > MinDifference ||
			 (Other.y - y) < -MinDifference ||
			 (Other.y - y) > MinDifference ||
			 (Other.z - z) < -MinDifference ||
			 (Other.z - z) > MinDifference ||
			 (Other.w - w) < -MinDifference ||
			 (Other.w - w) > MinDifference );
}


template<typename TYPE> template<typename OTHERTYPE> void Type2<TYPE>::operator=(const Type2<OTHERTYPE>& v)	{	x = (TYPE)v.x;	y = (TYPE)v.y;	}
template<typename TYPE> template<typename OTHERTYPE> void Type2<TYPE>::operator=(const Type3<OTHERTYPE>& v)	{	x = (TYPE)v.x;	y = (TYPE)v.y;	}
template<typename TYPE> template<typename OTHERTYPE> void Type2<TYPE>::operator=(const Type4<OTHERTYPE>& v)	{	x = (TYPE)v.x;	y = (TYPE)v.y;	}

template<typename TYPE> template<typename OTHERTYPE> void Type3<TYPE>::operator=(const Type2<OTHERTYPE>& v)	{	x = (TYPE)v.x;	y = (TYPE)v.y;	}
template<typename TYPE> template<typename OTHERTYPE> void Type3<TYPE>::operator=(const Type3<OTHERTYPE>& v)	{	x = (TYPE)v.x;	y = (TYPE)v.y;	z = (TYPE)v.z;	}
template<typename TYPE> template<typename OTHERTYPE> void Type3<TYPE>::operator=(const Type4<OTHERTYPE>& v)	{	x = (TYPE)v.x;	y = (TYPE)v.y;	z = (TYPE)v.z;	}

template<typename TYPE> template<typename OTHERTYPE> void Type4<TYPE>::operator=(const Type2<OTHERTYPE>& v)	{	x = (TYPE)v.x;	y = (TYPE)v.y;	}
template<typename TYPE> template<typename OTHERTYPE> void Type4<TYPE>::operator=(const Type3<OTHERTYPE>& v)	{	x = (TYPE)v.x;	y = (TYPE)v.y;	z = (TYPE)v.z;	}
template<typename TYPE> template<typename OTHERTYPE> void Type4<TYPE>::operator=(const Type4<OTHERTYPE>& v)	{	x = (TYPE)v.x;	y = (TYPE)v.y;	z = (TYPE)v.z;	w = (TYPE)v.w;	}


