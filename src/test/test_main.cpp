#include <clocale>
#include <gtest/gtest.h>

#include "../utils/chinese.h"
#include "../utils/helper.h"
#include <exception>
#include <gtest/internal/gtest-port.h>

const static std::string file_path = "./ansi.txt";

std::string ReadFile(const std::string& file_path)
{
    std::string content;
    Xzm::util::read_file(file_path, content);
    //std::cout << "read_file[" << content << "]" << std::endl;
    if (content.empty()) { throw std::exception(); }
    return content;
}

bool GBK2UTF8(const std::string& file_path)
{
    std::string str_ansi = ReadFile(file_path);
    if (str_ansi.empty()) {
        return false;
    }
    std::cout << "str_ansi:" << str_ansi << std::endl;
    std::string str_utf8;
    str_utf8 = Xzm::util::Chinese::instance()->GBKToUTF8(str_ansi);
    if (str_utf8.empty()) {
        std::cout << "str_utf8 is empty!" << std::endl;
        return false;
    }
    std::cout << "str_utf8:" << str_utf8 << std::endl;
    return true;
}

TEST(TestEncode, ReadFile)
{
    ASSERT_NO_THROW(ReadFile(file_path));
}

TEST(TestEncode, convert)
{
    ASSERT_EQ(GBK2UTF8(file_path), true);
}

int main(int argc, char** argv)
{
    setlocale(LC_CTYPE, "zh-CN");    // 使ansi编码有效,用于utf8和ansi转码,但是所有lib都回收影响,有风险
    testing::InitGoogleTest(&argc, argv);
    //return RUN_ALL_TESTS();
    GBK2UTF8(file_path);
}