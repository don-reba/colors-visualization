#include "RenderMesh.h"

#include <boost/test/unit_test.hpp>
#include <Eigen/Geometry>

using namespace Eigen;
using namespace std;

BOOST_AUTO_TEST_CASE(RenderMesh_Transform_Identity)
{
	BOOST_CHECK_EQUAL
		( ::Transform(Matrix4f::Identity(), Vector3f(1.0f, 2.0f, 3.0f))
		, Vector3f(1.0f, 2.0f, 3.0f)
		);
}

BOOST_AUTO_TEST_CASE(RenderMesh_Tranform_Scale)
{
	BOOST_CHECK_EQUAL
		( ::Transform(Affine3f(Translation3f(4.0f, 5.0f, 6.0f)).matrix(), Vector3f(1.0f, 2.0f, 3.0f))
		, Vector3f(5.0f, 7.0f, 9.0f)
		);
}