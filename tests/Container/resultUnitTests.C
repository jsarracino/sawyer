#include <Sawyer/Result.h>

#include <Sawyer/Assert.h>

using namespace Sawyer;

static void test01(const Result<int, std::string> &result) {
    ASSERT_always_require(result.isOk());
    ASSERT_always_require(result);
    ASSERT_always_require(!result.isError());
    ASSERT_always_require(result.ok());
    ASSERT_always_require(*result.ok() == 5);
    ASSERT_always_require(!result.error());
    ASSERT_always_require(result.expect("failed") == 5);
    ASSERT_always_require(result.unwrap() == 5);
    ASSERT_always_require(*result == 5);
    ASSERT_always_require(result.orElse(6) == 5);
    ASSERT_always_require(result.orDefault() == 5);
    ASSERT_always_require(result.orThrow() == 5);

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
    ASSERT_always_require(result.and_(a));
    ASSERT_always_require(*result.and_(a) == &x);

    Result<int, long*> b = Ok(6);
    ASSERT_always_require(result.or_(b));
    ASSERT_always_require(*result.or_(b) == 5);

    ASSERT_always_require(result.contains(5L));
    ASSERT_always_require(!result.containsError(""));
}

static void test02(const Result<int, std::string> &result) {
    ASSERT_always_require(!result.isOk());
    ASSERT_always_require(!result);
    ASSERT_always_require(result.isError());
    ASSERT_always_require(!result.ok());
    ASSERT_always_require(result.error());
    ASSERT_always_require(*result.error() == "error");

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
    ASSERT_always_require(!result.and_(a));
    ASSERT_always_require(result.and_(a).error());
    ASSERT_always_require(result.and_(a).unwrapError() == std::string("error"));

    Result<int, long*> b = Ok(6);
    ASSERT_always_require(result.or_(b));
    ASSERT_always_require(*result.or_(b) == 6);

    ASSERT_always_require(!result.contains(5L));
    ASSERT_always_require(result.containsError("error"));
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
    ASSERT_always_require(result.and_(a));
    ASSERT_always_require(*result.and_(a) == &x);

    Result<std::string, long*> b = Ok("yep");
    ASSERT_always_require(result.or_(b));
    ASSERT_always_require(*result.or_(b) == std::string("ok"));

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
    ASSERT_always_require(!result.and_(a));
    ASSERT_always_require(result.and_(a).error());
    ASSERT_always_require(result.and_(a).unwrapError() == error);

    Result<std::string, long*> b = Ok("hi");
    ASSERT_always_require(result.or_(b));
    ASSERT_always_require(*result.or_(b) == std::string("hi"));

    ASSERT_always_require(!result.contains("x"));
    ASSERT_always_require(result.containsError(error));
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
}
