#ifndef Sawyer_Result_H
#define Sawyer_Result_H

#include <Sawyer/Optional.h>

#include <boost/lexical_cast.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/variant.hpp>
#include <exception>
#include <string>
#include <type_traits>

namespace Sawyer {

/** Success value. */
template<class T>
class Ok {
public:
    using Value = T;

private:
    Value ok_;

private:
    friend class boost::serialization::access;

    template<class S>
    void serialize(S &s, const unsigned /*version*/) {
        s & BOOST_SERIALIZATION_NVP(ok_);
    }

public:
    Ok() = delete;

    /** Copy constructor. */
    explicit Ok(const Ok &other)
        : ok_(other.ok_) {}

    /** Construct from an value. */
    explicit Ok(const Value &ok)
        : ok_(ok) {}

    /** Assignment.
     *
     * @{ */
    Ok& operator=(const Ok &other) {
        ok_ = other.ok_;
        return *this;
    }
    Ok& operator=(const Value &ok) {
        ok_ = ok;
        return *this;
    }
    /** @} */

    /** Dereference to obtain value. */
    const Value& operator*() const {
        return ok_;
    }

    /** Dereference to obtain pointer. */
    const Value* operator->() const {
        return &ok_;
    }
};

// Specialization for Ok that stores string literals, as in Ok("foo"). These get treated as std::string instead.
template<size_t N>
class Ok<char[N]> {
public:
    using Value = std::string;

private:
    std::string ok_;

private:
    friend class boost::serialization::access;

    template<class S>
    void serialize(S &s, const unsigned /*version*/) {
        s & BOOST_SERIALIZATION_NVP(ok_);
    }

public:
    Ok() = delete;

    explicit Ok(const Value &s)
        : ok_(s) {}

    Ok& operator=(const Ok &other) {
        ok_ = other.ok_;
        return *this;
    }

    Ok& operator=(const Value &ok) {
        ok_ = ok;
        return *this;
    }

    const Value& operator*() const {
        return ok_;
    }

    const Value* operator->() const {
        return &ok_;
    }
};

/** Error value. */
template<class E>
class Error {
public:
    using Value = E;

private:
    Value error_;

private:
    friend class boost::serialization::access;

    template<class S>
    void serialize(S &s, const unsigned /*version*/) {
        s & BOOST_SERIALIZATION_NVP(error_);
    }

public:
    Error() = delete;

    /** Copy constructor. */
    explicit Error(const Error &other)
        : error_(other.error_) {}

    /** Construct from a value. */
    explicit Error(const E &error)
        : error_(error) {}

    /** Assignment.
     *
     * @{ */
    Error& operator=(const Error &other) {
        error_ = other.error_;
        return *this;
    }
    Error& operator=(const Value &error) {
        error_ = error;
        return *this;
    }
    /** @} */

    /** Dereference to obtain error. */
    const Value& operator*() const {
        return error_;
    }

    /** Dereference to obtain pointer to error. */
    const Value* operator->() const {
        return &error_;
    }
};

// Specialization for Error that stores string literals as in Error("syntax error"). It stores them as std::string instead.
template<size_t N>
class Error<char[N]> {
public:
    using Value = std::string;

private:
    std::string error_;

private:
    friend class boost::serialization::access;

    template<class S>
    void serialize(S &s, const unsigned /*version*/) {
        s & BOOST_SERIALIZATION_NVP(error_);
    }

public:
    Error() = delete;

    /** Copy constructor. */
    explicit Error(const Error &other)
        : error_(other.error_) {}

    /** Construct from a value. */
    explicit Error(const Value &error)
        : error_(error) {}

    /** Assignment.
     *
     * @{ */
    Error& operator=(const Error &other) {
        error_ = other.error_;
        return *this;
    }
    Error& operator=(const Value &error) {
        error_ = error;
        return *this;
    }
    /** @} */

    /** Dereference to obtain error. */
    const Value& operator*() const {
        return error_;
    }

    /** Dereference to obtain pointer to error. */
    const Value* operator->() const {
        return &error_;
    }
};

/** Result containing a value or an error. */
template<class T, class E>
class Result {
public:
    using OkValue = T;
    using ErrorValue = E;
    using OkType = Ok<T>;
    using ErrorType = Error<E>;

private:
    boost::variant<Ok<T>, Error<E>> result_;

private:
    friend class boost::serialization::access;

    template<class S>
    void save(S &s, const unsigned /*version*/) const {
        s <<boost::serialization::make_nvp("isOk", isOk());
        if (isOk()) {
            s <<boost::serialization::make_nvp("ok", unwrap());
        } else {
            s <<boost::serialization::make_nvp("error", unwrapError());
        }
    }

    template<class S>
    void load(S &s, const unsigned /*version*/) {
        bool isOk;
        s >>boost::serialization::make_nvp("isOk", isOk);
        if (isOk) {
            T ok;
            s >>boost::serialization::make_nvp("ok", ok);
            result_ = Ok(ok);
        } else {
            E error;
            s >>boost::serialization::make_nvp("error", error);
            result_ = Error(error);
        }
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER();

public:
    template<class U = T>
    /*implicit*/ Result(const Ok<U> &ok)
        : result_(Ok(*ok)) {}

    template<class F = E>
    /*implicit*/ Result(const Error<F> &error)
        : result_(Error(*error)) {}

    template<class U = T>
    Result& operator=(const Ok<U> &ok) {
        result_ = Ok(*ok);
        return *this;
    }

    template<class F = E>
    Result& operator=(const Error<F> &error) {
        result_ = Error(*error);
        return *this;
    }

    /** Returns true if the result is okay.
     *
     * @{ */
    bool isOk() const {
        return result_.which() == 0;
    }
    operator bool() const {
        return isOk();
    }
    /** @} */

    /** Returns true if the result is an error. */
    bool isError() const {
        return !isOk();
    }

    /** Convert to Optional<T>.
     *
     *  If this result is okay, then return the result, otherwise return nothing. */
    const Sawyer::Optional<T> ok() const {
        if (isOk()) {
            return *boost::get<Ok<T>>(result_);
        } else {
            return Sawyer::Nothing();
        }
    }

    /** Convert to Optional<E>.
     *
     *  If this result is an error, then return the error, otherwise return nothing. */
    const Sawyer::Optional<E> error() const {
        if (isOk()) {
            return Sawyer::Nothing();
        } else {
            return *boost::get<Error<E>>(result_);
        }
    }

    /** Returns the success value or throws an exception.
     *
     *  If this result is okay, then returns its value, otherwise throws an <code>std::runtime_error</code> with the specified string. */
    const T& expect(const std::string &mesg) const {
        if (isOk()) {
            return *boost::get<Ok<T>>(result_);
        } else {
            throw std::runtime_error(mesg);
        }
    }

    /** Returns the success value or throws an exception.
     *
     *  If this result is okay, then returns its value, otherwise throws an <code>std::runtime_error</code>.
     *
     * @{ */
    const T& unwrap() const {
        return expect("result is not okay");
    }
    const T& operator*() const {
        return unwrap();
    }
    /** @} */

    /** Returns the contained @ref Ok value or a provided default. */
    const T orElse(const T &dflt) const {
        return isOk() ? unwrap() : dflt;
    }

    /** Returns the okay value or a default constructed value. */
    const T& orDefault() const {
        static T dflt = T();
        return isOk() ? unwrap() : dflt;
    }

    /** Returns the value or throws an exception.
     *
     *  If the result is okay, then its value is returned, otherwise either the error is thrown or it is used to construct the
     *  specified @p Exception which is then thrown. */
    template<class Exception = E>
    const T& orThrow() const {
        if (isOk()) {
            return unwrap();
        } else {
            throw Exception(*error());
        }
    }

    /** Returns the value or throws an exception constructed from the specified value. */
    template<class Exception = E>
    const T& orThrow(const Exception &e) const {
        if (isOk()) {
            return unwrap();
        } else {
            throw e;
        }
    }

    /** Returns the error value or throws an exception.
     *
     *  If this result is an error, then returns the error, otherwise throws an <code>std::runtime_error</code> with the specified string. */
    const E& expectError(const std::string &mesg) const {
        if (isOk()) {
            throw std::runtime_error(mesg);
        } else {
            return *boost::get<Error<E>>(result_);
        }
    }

    /** Returns the error value or throws an exception.
     *
     *  If this result is an error, then returns the error, otherwise throws an <code>std::runtime_error</code>. */
    const E& unwrapError() const {
        return expectError("result is not an error");
    }

    /** Returns this error or the other result.
     *
     *  If this result is okay, then returns @p other. Otherwise returns the error value of this result. */
    template<class U>
    const Result<U, E> and_(const Result<U, E> &other) const {
        if (isOk()) {
            return other;
        } else {
            return boost::get<Error<E>>(result_);
        }
    }

    /** Returns this value or the other result.
     *
     *  If this result is okay then return it, otherwise returns the @p other result. */
    template<class F>
    const Result<T, F> or_(const Result<T, F> &other) const {
        if (isOk()) {
            return boost::get<Ok<T>>(result_);
        } else {
            return other;
        }
    }

    /** Returns true if this result contains the specified okay value. */
    template<class U>
    bool contains(const U &value) const {
        return isOk() ? unwrap() == value : false;
    }

    /** Returns true if this result contains the specified error value. */
    template<class F>
    bool containsError(const F &error) const {
        return isOk() ? false : unwrapError() == error;
    }

#if 0 // [Robb Matzke 2022-08-17]
    /** Transposes a @ref Result of an @ref Optional to an @ref Optional @ref Result.
     *
     *  If this result is an empty @ref Optional then an empty @ref Optional is returned. If this result is a non-empty @ref
     *  Optional then a non-empty @ref Optional containing the @ref Ok result is returned. Otherwise a non-empty @ref Optional
     *  containing the @ref Error is returned. */
    Optional<Result<typename T::Value, E>>
    transpose() const {
        if (isOk()) {
            if (unwrap().isEmpty()) {
                return Sawyer::Nothing();
            } else {
                return Ok(*unwrap());
            }
        } else {
            return Error(unwrapError());
        }
    }

    /** Flatten a result of a result to a result. */
    Result<typename OkValue::OkValue, E> flatten() const {
        if (isOk()) {
            if (unwrap().isOk()) {
                return OkValue::OkType(unwrap().unwrap());
            } else {
                return Error(unwrap().unwrapError());
            }
        } else {
            return Error(unwrapError());
        }
    }
#endif

    /** Conditionally save a value.
     *
     *  If this result has a value, then assign it to the argument and return true, otherwise do nothing and return false. */
    template<class U>
    bool assignTo(U &out) const {
        if (isOk()) {
            out = unwrap();
            return true;
        } else {
            return false;
        }
    }
};

} // namespace
#endif
