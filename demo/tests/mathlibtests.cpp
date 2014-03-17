#include <gtest/gtest.h>

#include "vector.h"
#include <stdlib.h>
#include <time.h>

class MathlibTests : public testing::Test
{
public:
	void SetUp() 
	{
		srand((unsigned int)time(0));
	}

	void TearDown() 
	{
	}

	float randomFloat(float p1 = -10.0f, float p2 = 10.0f)
	{
		float k = (float)(rand() % 1000) / (float)(1000 - 1);
		return p1 * (1.0f - k) + p2 * k;
	}
};

TEST_F(MathlibTests, Vector3)
{
	vector3 vec;
	ASSERT_FLOAT_EQ(vec.x, 0.0f);
	ASSERT_FLOAT_EQ(vec.y, 0.0f);
	ASSERT_FLOAT_EQ(vec.z, 0.0f);

	vec = vector3(randomFloat(), randomFloat(), randomFloat());
	vec.norm();
	ASSERT_FLOAT_EQ(vec.len(), 1.0f);

	float angle = vector3::angle(vec, vec);
	ASSERT_FLOAT_EQ(angle, 0.0f);

	angle = vector3::angle(vec, -vec);
	ASSERT_FLOAT_EQ(angle, N_PI);
}