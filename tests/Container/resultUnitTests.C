#include <Sawyer/Result.h>

#include <Sawyer/Assert.h>
#include <iostream>
#include <cmath>

using namespace Sawyer;

static void test01(const Result<int, std::string> &result) {
    ASSERT_always_require(result.isOk());
    ASSERT_always_require(result);
    ASSERT_always_require(!result.isError());
    ASSERT_always_require(result.ok());
    ASSERT_always_require(*result.ok() == 5);
    ASSERT_always_require(result == Ok(5));
    ASSERT_always_require(result != Ok(6));
    ASSERT_always_require(result != Error("foo"));
    ASSERT_always_require(!result.error());
    ASSERT_always_require(result.expect("failed") == 5);
    ASSERT_always_require(result.unwrap() == 5);
    ASSERT_always_require(*result == 5);
    ASSERT_always_require(result.orElse(6) == 5);
    ASSERT_always_require(result.orDefault() == 5);
    ASSERT_always_require(result.orThrow() == 5);
    ASSERT_always_require(result == result);
    ASSERT_always_require(!(result != result));

    try {
        result.expectError("foo");
        ASSERT_not_reachable("expectError succeeded when it should have failed");
    } catch (const std::runtime_error &e) {
        ASSERT_always_require(e.what() == std::string("foo"));
    }

    try {
        result.unwrapError();
        ASSERT_not_reachable("unwrapError succeeded when it should have failed");
    } catch (const std::runtime_error &e) {
        ASSERT_always_require(e.what() == std::string("result is not an error"));
    }

    long x = 0;
    Result<long*, std::string> a = Ok(&x);
    ASSERT_always_require(result.andThen(a));
    ASSERT_always_require(*result.andThen(a) == &x);

    Result<int, long*> b = Ok(6);
    ASSERT_always_require(result.orElse(b));
    ASSERT_always_require(*result.orElse(b) == 5);

    ASSERT_always_require(result.contains(5L));
    ASSERT_always_require(!result.containsError(""));

    Result<int, std::string> r1 = result.orElse([](const std::string &error) {
            return Error(error + "2");
        });
    ASSERT_always_require(*r1.ok() == 5);

    Result<int, std::string> r2 = result.andThen([](int i) {
            return Ok(i * 2);
        });
    ASSERT_always_require(r2 == Ok(10));

    Result<int, std::string> r3 = result.andThen([](int i) {
            return Error("too small");
        });
    ASSERT_always_require(r3 == Error("too small"));
}

static void test02(const Result<int, std::string> &result) {
    ASSERT_always_require(!result.isOk());
    ASSERT_always_require(!result);
    ASSERT_always_require(result.isError());
    ASSERT_always_require(!result.ok());
    ASSERT_always_require(result.error());
    ASSERT_always_require(*result.error() == "error");
    ASSERT_always_require(result != Ok(5));
    ASSERT_always_require(result == Error("error"));
    ASSERT_always_require(result != Error("foo"));

    try {
        result.expect("foo");
        ASSERT_not_reachable("expect succeeded when it should have failed");
    } catch (const std::runtime_error &e) {
        ASSERT_always_require(e.what() == std::string("foo"));
    }

    try {
        result.unwrap();
        ASSERT_not_reachable("unwrap succeeded when it should have failed");
    } catch (const std::runtime_error &e) {
        ASSERT_always_require(e.what() == std::string("result is not okay"));
    }

    try {
        *result;
        ASSERT_not_reachable("operator* succeeded when it should have failed");
    } catch (const std::runtime_error &e) {
        ASSERT_always_require(e.what() == std::string("result is not okay"));
    }

    ASSERT_always_require(result.orElse(6) == 6);
    ASSERT_always_require(result.orDefault() == 0);

    try {
        result.orThrow();
        ASSERT_not_reachable("orThrow succeeded when it should have failed");
    } catch (const std::string &e) {
        ASSERT_always_require(e == "error");
    }

    try {
        result.orThrow<std::runtime_error>();
        ASSERT_not_reachable("orThrow succeeded when it should have failed");
    } catch (const std::runtime_error &e) {
        ASSERT_always_require(e.what() == std::string("error"));
    }

    try {
        result.orThrow(1);
        ASSERT_not_reachable("orThrow succeeded when it should have failed");
    } catch (int e) {
        ASSERT_always_require(e == 1);
    }

#if 0 // [Robb Matzke 2022-08-17]
    try {
        result.orThrow<std::runtime_error>("foo");
        ASSERT_not_reachable("orThrow succeeded when it should have failed");
    } catch (const std::runtime_error &e) {
        ASSERT_always_require(e.what() == std::string("foo"));
    }
#endif

    ASSERT_always_require(result.expectError("foo") == std::string("error"));
    ASSERT_always_require(result.unwrapError() == std::string("error"));

    long x = 0;
    Result<long*, std::string> a = Ok(&x);
    ASSERT_always_require(!result.andThen(a));
    ASSERT_always_require(result.andThen(a).error());
    ASSERT_always_require(result.andThen(a).unwrapError() == std::string("error"));

    Result<int, long*> b = Ok(6);
    ASSERT_always_require(result.orElse(b));
    ASSERT_always_require(*result.orElse(b) == 6);

    ASSERT_always_require(!result.contains(5L));
    ASSERT_always_require(result.containsError("error"));

    Result<int, std::string> r1 = result.orElse([](const std::string &error) {
            return Error(error + "-2");
        });
    ASSERT_always_require(*r1.error() == std::string("error-2"));

    Result<int, std::string> r2 = result.orElse([](const std::string &error) {
            return Ok((int)error.size() * 10);
        });
    ASSERT_always_require(*r2.ok() == 50);
}

struct LocationError {
    std::string mesg;
    std::string fileName;
    size_t line = 0;

    bool operator==(const LocationError &other) const {
        return mesg == other.mesg && fileName == other.fileName && line == other.line;
    }
};

using LocationResult = Result<std::string, LocationError>;

static void test03(const LocationResult &result) {
    ASSERT_always_require(result.isOk());
    ASSERT_always_require(result);
    ASSERT_always_require(!result.isError());
    ASSERT_always_require(result.ok());
    ASSERT_always_require(*result.ok() == std::string("ok"));
    ASSERT_always_require(!result.error());
    ASSERT_always_require(result.expect("failed") == std::string("ok"));
    ASSERT_always_require(result.unwrap() == std::string("ok"));
    ASSERT_always_require(*result == std::string("ok"));
    ASSERT_always_require(result.orElse("nope") == std::string("ok"));
    ASSERT_always_require(result.orDefault() == std::string("ok"));
    ASSERT_always_require(result.orThrow() == std::string("ok"));
    ASSERT_always_require(result == Ok("ok"));
    ASSERT_always_require(result != Error(LocationError()));

    try {
        result.expectError("foo");
        ASSERT_not_reachable("expectError succeeded when it should have failed");
    } catch (const std::runtime_error &e) {
        ASSERT_always_require(e.what() == std::string("foo"));
    }

    try {
        result.unwrapError();
        ASSERT_not_reachable("unwrapError succeeded when it should have failed");
    } catch (const std::runtime_error &e) {
        ASSERT_always_require(e.what() == std::string("result is not an error"));
    }

    long x = 0;
    Result<long*, LocationError> a = Ok(&x);
    ASSERT_always_require(result.andThen(a));
    ASSERT_always_require(*result.andThen(a) == &x);

    Result<std::string, long*> b = Ok("yep");
    ASSERT_always_require(result.orElse(b));
    ASSERT_always_require(*result.orElse(b) == std::string("ok"));

    ASSERT_always_require(result.contains("ok"));
    ASSERT_always_require(!result.containsError(LocationError()));
}

static void test04(const LocationResult &result, const LocationError &error) {
    ASSERT_always_require(!result.isOk());
    ASSERT_always_require(!result);
    ASSERT_always_require(result.isError());
    ASSERT_always_require(!result.ok());
    ASSERT_always_require(result.error());
    ASSERT_always_require(*result.error() == error);
    ASSERT_always_require(result != Ok("foo"));
    ASSERT_always_require(result == Error(error));

    try {
        result.expect("foo");
        ASSERT_not_reachable("expect succeeded when it should have failed");
    } catch (const std::runtime_error &e) {
        ASSERT_always_require(e.what() == std::string("foo"));
    }

    try {
        result.unwrap();
        ASSERT_not_reachable("unwrap succeeded when it should have failed");
    } catch (const std::runtime_error &e) {
        ASSERT_always_require(e.what() == std::string("result is not okay"));
    }

    try {
        *result;
        ASSERT_not_reachable("operator* succeeded when it should have failed");
    } catch (const std::runtime_error &e) {
        ASSERT_always_require(e.what() == std::string("result is not okay"));
    }

    ASSERT_always_require(result.orElse("abc") == std::string("abc"));
    ASSERT_always_require(result.orDefault() == std::string());

    try {
        result.orThrow();
        ASSERT_not_reachable("orThrow succeeded when it should have failed");
    } catch (const LocationError &e) {
        ASSERT_always_require(e == error);
    }

    try {
        result.orThrow(1);
        ASSERT_not_reachable("orThrow succeeded when it should have failed");
    } catch (int e) {
        ASSERT_always_require(e == 1);
    }

#if 0 // [Robb Matzke 2022-08-17]
    try {
        result.orThrow<std::runtime_error>("foo");
        ASSERT_not_reachable("orThrow succeeded when it should have failed");
    } catch (const std::runtime_error &e) {
        ASSERT_always_require(e.what() == std::string("foo"));
    }
#endif

    ASSERT_always_require(result.expectError("foo") == error);
    ASSERT_always_require(result.unwrapError() == error);

    long x = 0;
    Result<long*, LocationError> a = Ok(&x);
    ASSERT_always_require(!result.andThen(a));
    ASSERT_always_require(result.andThen(a).error());
    ASSERT_always_require(result.andThen(a).unwrapError() == error);

    Result<std::string, long*> b = Ok("hi");
    ASSERT_always_require(result.orElse(b));
    ASSERT_always_require(*result.orElse(b) == std::string("hi"));

    ASSERT_always_require(!result.contains("x"));
    ASSERT_always_require(result.containsError(error));
    ASSERT_always_require(result.error()->line == 123);
}

enum class ErrorType { DOMAIN, LIMIT };

Result<double, ErrorType> squareRoot(double x) {
    if (x < 0) {
        return Error(ErrorType::DOMAIN);
    } else {
        return Ok(sqrt(x));
    }
}

static void test05() {
    const Result<double, ErrorType> r1 = Ok(25.0);
    ASSERT_always_require(r1.andThen(squareRoot) == Ok(5.0));

    const Result<double, ErrorType> r2 = Ok(-1.0);
    ASSERT_always_require(r2.andThen(squareRoot) == Error(ErrorType::DOMAIN));

    const Result<double, ErrorType> r3 = Error(ErrorType::LIMIT);
    ASSERT_always_require(r3.andThen(squareRoot) == Error(ErrorType::LIMIT));
}

int main() {
    Result<int, std::string> result = Ok(5);
    test01(result);

    result = Error("error");
    test02(result);

    LocationResult result2 = Ok("ok");
    test03(result2);

    LocationError locErr;
    locErr.mesg = "an error";
    locErr.fileName = "a file";
    locErr.line = 123;
    result2 = Error(locErr);
    test04(result2, locErr);

    test05();
}
