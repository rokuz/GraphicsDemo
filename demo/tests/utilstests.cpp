#include <gtest/gtest.h>

#include "utils.h"

class UtilsTests : public testing::Test
{
public:
	void SetUp() 
	{
	}

	void TearDown() 
	{
	}
};

TEST_F(UtilsTests, Tokenize)
{
	auto result1 = utils::Utils::tokenize<std::wstring>(L"\n", L'\n');
	ASSERT_TRUE(result1.empty());
	auto result1_1 = utils::Utils::tokenize<std::wstring>(L"", L'\n');
	ASSERT_TRUE(result1_1.empty());

	auto result2 = utils::Utils::tokenize<std::wstring>(L"\n\n", L'\n');
	ASSERT_TRUE(result2.empty());

	auto result3 = utils::Utils::tokenize<std::wstring>(L"\nt1\n", L'\n');
	ASSERT_EQ(result3.size(), 1);
	ASSERT_EQ((*result3.begin()).first, 1);
	ASSERT_EQ((*result3.begin()).second, 2);
	auto result3_1 = utils::Utils::tokenize<std::wstring>(L"\nt1", L'\n');
	ASSERT_EQ(result3_1.size(), 1);
	ASSERT_EQ((*result3_1.begin()).first, 1);
	ASSERT_EQ((*result3_1.begin()).second, 2);

	auto result4 = utils::Utils::tokenize<std::wstring>(L"\n\nt\n\n", L'\n');
	ASSERT_EQ(result4.size(), 1);
	ASSERT_EQ((*result4.begin()).first, 2);
	ASSERT_EQ((*result4.begin()).second, 2);

	auto result5 = utils::Utils::tokenize<std::wstring>(L"\n\nt1\nt\nt2\n\nt3", L'\n');
	ASSERT_EQ(result5.size(), 4);
	auto it = result5.begin();
	ASSERT_EQ((*it).first, 2);
	ASSERT_EQ((*it).second, 3);
	it++;
	ASSERT_EQ((*it).first, 5);
	ASSERT_EQ((*it).second, 5);
	it++;
	ASSERT_EQ((*it).first, 7);
	ASSERT_EQ((*it).second, 8);
	it++;
	ASSERT_EQ((*it).first, 11);
	ASSERT_EQ((*it).second, 12);

	auto result6 = utils::Utils::tokenize<std::wstring>(L"a", L'\n');
	ASSERT_EQ(result6.size(), 1);
	ASSERT_EQ((*result6.begin()).first, 0);
	ASSERT_EQ((*result6.begin()).second, 0);

	auto result7 = utils::Utils::tokenize<std::wstring>(L"abc", L'\n');
	ASSERT_EQ(result7.size(), 1);
	ASSERT_EQ((*result7.begin()).first, 0);
	ASSERT_EQ((*result7.begin()).second, 2);

	auto result8 = utils::Utils::tokenize<std::wstring>(L"abc\n", L'\n');
	ASSERT_EQ(result8.size(), 1);
	ASSERT_EQ((*result8.begin()).first, 0);
	ASSERT_EQ((*result8.begin()).second, 2);
}