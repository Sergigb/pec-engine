/******************************************************************************\
| OpenGL 4 Example Code.                                                       |
| Accompanies written series "Anton's OpenGL 4 Tutorials"                      |
| Email: anton at antongerdelan dot net                                        |
| First version 27 Jan 2014                                                    |
| Copyright Dr Anton Gerdelan, Trinity College Dublin, Ireland.                |
| See individual libraries' separate legal notices                             |
|******************************************************************************|
| Commonly-used maths structures and functions                                 |
| Simple-as-possible. No disgusting templates.                                 |
| Structs vec3, mat4, versor. just hold arrays of floats called "v","m","q",   |
| respectively. So, for example, to get values from a mat4 do: my_mat.m        |
| A versor is the proper name for a unit quaternion.                           |
| This is C++ because it's sort-of convenient to be able to use maths operators|
\******************************************************************************/
#ifndef MATHS_FUNCS_HPP
#define MATHS_FUNCS_HPP

namespace math{
    struct vec2;
    struct vec3;
    struct vec4;
    struct versor;

    struct vec2 {
        vec2();
        vec2( float x, float y );
        float v[2];
    };

    struct vec3 {
        vec3();
        // create from 3 scalars
        vec3( float x, float y, float z );
        // create from vec2 and a scalar
        vec3( const vec2 &vv, float z );
        // create from truncated vec4
        vec3( const vec4 &vv );
        // fix for Wdeprecated-copy warning in g++9/clang
        vec3( const vec3 &rhs );
        // add vector to vector
        vec3 operator+( const vec3 &rhs ) const;
        // add scalar to vector
        vec3 operator+( float rhs ) const;
        // because user's expect this too
        vec3 &operator+=( const vec3 &rhs );
        // subtract vector from vector
        vec3 operator-( const vec3 &rhs ) const;
        // add vector to vector
        vec3 operator-( float rhs ) const;
        // because users expect this too
        vec3 &operator-=( const vec3 &rhs );
        // multiply with scalar
        vec3 operator*( float rhs ) const;
        // because users expect this too
        vec3 &operator*=( float rhs );
        // divide vector by scalar
        vec3 operator/( float rhs ) const;
        // because users expect this too
        vec3 &operator=( const vec3 &rhs );

        // internal data
        float v[3];
    };

    struct vec4 {
        vec4();
        vec4( float x, float y, float z, float w );
        vec4( const vec2 &vv, float z, float w );
        vec4( const vec3 &vv, float w );
        vec4 operator/( float rhs ) const;
        vec4 operator*( float rhs ) const;
        vec4 operator+( float rhs ) const;
        vec4 operator+( const vec4 &rhs ) const;
        vec4 operator-( float rhs ) const;
        vec4 operator-( const vec4 &rhs ) const;

        float v[4];
    };

    /* stored like this:
    a d g
    b e h
    c f i */
    struct mat3 {
        mat3();
        mat3( float a, float b, float c, float d, float e, float f, float g, float h,
                    float i );
        vec3 operator*( const vec3 &rhs ) const;
        mat3 operator*( const mat3 &rhs ) const;
        float m[9];
    };

    /* stored like this:
    0 4 8    12
    1 5 9    13
    2 6 10 14
    3 7 11 15*/
    struct mat4 {
        mat4();
        // note! this is entering components in ROW-major order
        mat4( float a, float b, float c, float d, float e, float f, float g, float h,
                    float i, float j, float k, float l, float mm, float n, float o, float p );
        mat4( const mat4 &rhs );
        vec4 operator*( const vec4 &rhs ) const;
        mat4 operator*( const mat4 &rhs ) const;
        mat4 &operator=( const mat4 &rhs );
        float m[16];
    };

    struct versor {
        versor();
        versor operator/( float rhs ) const;
        versor operator*( float rhs ) const;
        versor operator*( const versor &rhs ) const;
        versor operator+( const versor &rhs ) const;
        float q[4];
    };

    void print( const vec2 &v );
    void print( const vec3 &v );
    void print( const vec4 &v );
    void print( const mat3 &m );
    void print( const mat4 &m );
    // vector functions
    float length( const vec3 &v );
    float length2( const vec3 &v );
    float length( const vec4 &v );
    float length2( const vec4 &v );
    vec3 normalise( const vec3 &v );
    vec4 normalise(const vec4 &v );
    float dot( const vec3 &a, const vec3 &b );
    float dot( const vec4 &a, const vec3 &b );
    vec3 cross( const vec3 &a, const vec3 &b );
    float get_squared_dist( vec3 from, vec3 to );
    float direction_to_heading( vec3 d );
    vec3 heading_to_direction( float degrees );
    float distance(const vec2 &a, const vec2 &b);
    float distance(const vec3 &a, const vec3 &b);
    float distance(const vec4 &a, const vec4 &b);
    float distance_plane_point( const vec4 &plane, const vec3 &point );
    vec3 arb_perpendicular( const vec3& v );
    // matrix functions
    mat3 zero_mat3();
    mat3 identity_mat3();
    mat4 zero_mat4();
    mat4 identity_mat4();
    float determinant( const mat4 &mm );
    mat4 inverse( const mat4 &mm );
    mat4 transpose( const mat4 &mm );
    // affine functions
    mat4 translate( const mat4 &m, const vec3 &v );
    mat4 rotate_x_deg( const mat4 &m, float deg );
    mat4 rotate_y_deg( const mat4 &m, float deg );
    mat4 rotate_z_deg( const mat4 &m, float deg );
    mat4 scale( const mat4 &m, const vec3 &v );
    // camera functions
    mat4 look_at( const vec3 &cam_pos, vec3 targ_pos, const vec3 &up );
    mat4 perspective( float fovy, float aspect, float near, float far );
    mat4 orthographic( float right, float left, float top, float bottom, float far , float near);
    // quaternion functions
    versor quat_from_axis_rad( float radians, float x, float y, float z );
    versor quat_from_axis_deg( float degrees, float x, float y, float z );
    mat4 quat_to_mat4( const versor &q );
    float dot( const versor &q, const versor &r );
    versor slerp( const versor &q, const versor &r );
    versor from_mat3( const mat3& m);
    // stupid overloading wouldn't let me use const
    versor normalise( versor &q );
    void print( const versor &q );
    versor slerp( versor &q, versor &r, float t );
    mat3 rotation_align( const vec3& d, const vec3& z );
}


namespace dmath{ // double precision for the camera
    struct vec4;

    struct vec3 {
        vec3();
        vec3( double x, double y, double z );
        vec3( const vec3 &rhs );
        vec3( const vec4 &vv );
        vec3 operator+( const vec3 &rhs ) const;
        vec3 &operator+=( const vec3 &rhs );
        vec3 operator-( const vec3 &rhs ) const;
        vec3 operator-() const;
        vec3 &operator-=( const vec3 &rhs );
        vec3 operator*( double rhs ) const;
        vec3 &operator*=( double rhs );
        vec3 &operator=( const vec3 &rhs );
        vec3 operator/( double rhs ) const;

        // internal data
        double v[3];
    };

    struct vec4 {
        vec4();
        vec4( double x, double y, double z, double w );
        vec4( const vec4 &rhs );
        vec4( const vec3 &vv, double w );
        vec4 operator*( double rhs ) const;
        vec4 operator+( double rhs ) const;
        vec4 operator+( const vec4 &rhs ) const;
        vec4 operator-( double rhs ) const;
        vec4 operator-( const vec4 &rhs ) const;
        vec4 operator/( double rhs ) const;
        vec4 &operator=( const vec4 &rhs );

        double v[4];
    };

    struct mat4 {
        mat4();
        mat4( double a, double b, double c, double d, double e, double f, double g, double h,
              double i, double j, double k, double l, double mm, double n, double o, double p );
        mat4( const mat4 &rhs );
        vec4 operator*( const vec4 &rhs ) const;
        mat4 operator*( const mat4 &rhs ) const;
        mat4 &operator=( const mat4 &rhs );

        double m[16];
    };

    struct versor {
        versor();
        versor operator/( double rhs ) const;
        versor operator*( double rhs ) const;
        versor operator*( const versor &rhs ) const;
        versor operator+( const versor &rhs ) const;

        double q[4];
    };

    void print( const mat4& m );
    // vector functions
    vec3 normalise(const vec3 &v );
    vec4 normalise(const vec4 &v );
    double length( const vec3 &v );
    double length( const vec4 &v );
    vec3 cross( const vec3 &a, const vec3 &b );
    double distance(const vec3 &a, const vec3 &b);
    double dot( const vec3 &a, const vec3 &b );
    // matrix functions
    mat4 zero_mat4();
    mat4 identity_mat4();
    mat4 translate( const mat4 &m, const vec3 &v );
    mat4 inverse( const mat4 &mm );
    double determinant( const mat4 &mm );
    mat4 rotate_x_deg( const mat4 &m, double deg );
    mat4 rotate_y_deg( const mat4 &m, double deg );
    mat4 rotate_z_deg( const mat4 &m, double deg );
    mat4 rotate_axis( const mat4 &m, double deg, const vec3 &axis );
    mat4 identity_mat4();
    //versor functions
    mat4 quat_to_mat4( const versor &q );
    versor normalise( versor &q );
    versor quat_from_axis_rad( double radians, double x, double y, double z );
    versor from_mat3( const math::mat3& m );
}


#define TAU 2.0 * M_PI
#define ONE_DEG_IN_RAD ( 2.0 * M_PI ) / 360.0 // 0.017444444
#define ONE_RAD_IN_DEG 360.0 / ( 2.0 * M_PI ) // 57.2957795
    
#endif
