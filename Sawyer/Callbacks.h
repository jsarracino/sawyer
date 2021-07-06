#ifndef Sawyer_Callbacks_H
#define Sawyer_Callbacks_H

#include <Sawyer/Sawyer.h>
#include <Sawyer/SharedPointer.h>
#include <list>

namespace Sawyer {

// FIXME[Robb Matzke 2014-08-13]: documentation
template<class Callback>
class Callbacks {
private:
    typedef std::list<Callback> CbList;
    CbList callbacks_;

public:
    bool isEmpty() const {
        return callbacks_.empty();
    }

    Callbacks& append(const Callback &callback) {
        callbacks_.push_back(callback);
        return *this;
    }

    Callbacks& append(const Callbacks &other) {
        callbacks_.insert(callbacks_.end(), other.callbacks_.begin(), other.callbacks_.end());
        return *this;
    }
    
    Callbacks& prepend(const Callback &callback) {
        callbacks_.push_front(callback);
        return *this;
    }

    Callback& prepend(const Callbacks &other) {
        callbacks_.insert(callbacks_.begin(), other.callbacks_.begin(), other.callbacks_.end());
        return *this;
    }
    
    Callbacks& eraseFirst(const Callback &callback) {
        for (typename CbList::iterator iter=callbacks_.begin(); iter!=callbacks_.end(); ++iter) {
            if (*iter == callback) {
                callbacks_.erase(iter);
                break;
            }
        }
        return *this;
    }

    Callbacks& eraseLast(const Callback &callback) {
        for (typename CbList::reverse_iterator reverseIter=callbacks_.rbegin(); reverseIter!=callbacks_.rend(); ++reverseIter) {
            if (*reverseIter == callback) {
                typename CbList::iterator forwardIter = (++reverseIter).base();
                callbacks_.erase(forwardIter);
                break;
            }
        }
        return *this;
    }

    Callbacks& eraseMatching(const Callback &callback) {
        typename CbList::iterator iter = callbacks_.begin();
        while (iter!=callbacks_.end()) {
            if (*iter == callback) {
                typename CbList::iterator toErase = iter++;
                callbacks_.erase(toErase);              // std::list iterators are stable over erasure
            } else {
                ++iter;
            }
        }
    }

    template<class CB, class Args>
    bool applyCallback(CB *callback, bool chained, Args &args) const {
        return (*callback)(chained, args);
    }

    template<class CB, class Args>
    bool applyCallback(const SharedPointer<CB> &callback, bool chained, Args &args) const {
        return (*callback)(chained, args);
    }

#if __cplusplus >= 201103ul
    template<class CB, class Args>
    bool applyCallback(const std::shared_ptr<CB> &callback, bool chained, Args &args) const {
        return (*callback)(chained, args);
    }
#endif

    template<class CB, class Args>
    bool applyCallback(CB &callback, bool chained, Args &args) const {
        return callback(chained, args);
    }

    template<class Arguments>
    bool apply(bool chained, const Arguments &arguments) const {
        for (typename CbList::const_iterator iter=callbacks_.begin(); iter!=callbacks_.end(); ++iter)
            chained = applyCallback(*iter, chained, arguments);
        return chained;
    }

    template<class Arguments>
    bool apply(bool chained, Arguments &arguments) const {
        for (typename CbList::const_iterator iter=callbacks_.begin(); iter!=callbacks_.end(); ++iter)
            chained = applyCallback(*iter, chained, arguments);
        return chained;
    }
};

template<class Callback>
class TemporaryCallback {
    Callbacks<Callback> &callbacks_;
    Callback callback_;
public:
    TemporaryCallback(Callbacks<Callback> &callbacks, const Callback &callback)
        : callbacks_(callbacks), callback_(callback) {
        callbacks_.append(callback);
    }

    ~TemporaryCallback() {
        callbacks_.eraseLast(callback_);
    }
};

} // namespace

#endif
