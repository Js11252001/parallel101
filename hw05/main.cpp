// 小彭老师作业05：假装是多线程 HTTP 服务器 - 富连网大厂面试官觉得很赞
#include <functional>
#include <chrono>
#include <shared_mutex>
#include <future>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <string>
#include <thread>
#include <map>


struct User {
    std::string password;
    std::string school;
    std::string phone;
};

std::map<std::string, User> users;
std::map<std::string, std::chrono::steady_clock::time_point> has_login;  // 换成 std::chrono::seconds 之类的
std::shared_mutex smtx;

//write
// 作业要求1：把这些函数变成多线程安全的
// 提示：能正确利用 shared_mutex 加分，用 lock_guard 系列加分
std::string do_register(std::string username, std::string password, std::string school, std::string phone) {
    std::unique_lock grd(smtx);
    User user = {password, school, phone};
    if (users.emplace(username, user).second)
        return "register succeed";
    else
        return "username has been regist";
}

//write
std::string do_login(std::string username, std::string password) {
    // 作业要求2：把这个登录计时器改成基于 chrono 的
    std::unique_lock grd(smtx);
    auto now = std::chrono::steady_clock::now();
    if (has_login.find(username) != has_login.end()) {
        auto sec = now - has_login.at(username);
        int64_t d = std::chrono::duration_cast<std::chrono::seconds>(sec).count();
        return std::to_string(d) + "seconds logged in within ";
    }
    has_login[username] = now;
    if (users.find(username) == users.end())
        return "Username error";
    if (users.at(username).password != password)
        return "password error";
    return "log succeed";
}

//read
std::string do_queryuser(std::string username) {
    std::shared_lock grd(smtx);
    try
    {
        auto& user = users.at(username);
        std::stringstream ss;
        ss << "User: " << username << std::endl;
        ss << "School:" << user.school << std::endl;
        ss << "Phone: " << user.phone << std::endl;
        return ss.str();
    }
    catch (const std::exception&)
    {
        std::cerr << "not found user" << std::endl;
        return "";
    }

}


struct ThreadPool {
    std::vector<std::thread> pool;
    void create(std::function<void()> start) {
        // 作业要求3：如何让这个线程保持在后台执行不要退出？
        // 提示：改成 async 和 future 且用法正确也可以加分
        std::thread thr(start);
        pool.push_back(std::move(thr));
    }
    ~ThreadPool() {
        for (auto& thr : pool) {
            thr.join();
        }
    }
};

ThreadPool tpool;


namespace test {  // 测试用例？出水用力！
std::string username[] = {"zxx", "wxl", "pyb", "hym"};
std::string password[] = {"hellojob", "anti-job42", "cihou233", "reCihou_!"};
std::string school[] = {"985", "zju", "cbu", "mit"};
std::string phone[] = {"110", "119", "120", "12315"};
}

int main() {
    for (int i = 0; i < 100; i++) {
        tpool.create([&] {
            std::cout << do_register(test::username[rand() % 4], test::password[rand() % 4], test::school[rand() % 4], test::phone[rand() % 4]) << std::endl;
        });
        tpool.create([&] {
            std::cout << do_login(test::username[rand() % 4], test::password[rand() % 4]) << std::endl;
        });
        tpool.create([&] {
            std::cout << do_queryuser(test::username[rand() % 4]) << std::endl;
        });
    }

    // 作业要求4：等待 tpool 中所有线程都结束后再退出
    return 0;
}
