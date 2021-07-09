#ifndef Sawyer_Stack_H
#define Sawyer_Stack_H

#include <Sawyer/Assert.h>
#include <Sawyer/IndexedList.h>

namespace Sawyer {
namespace Container {

/** %Stack-based container.
 *
 *  The stack stores values in a last-in-first-out order. New items are pushed onto the top of the stack and existing items are
 *  popped from the top of the stack. */
template<typename T>
class Stack {
public:
    typedef T Value;
private:
    IndexedList<T> items_;
public:
    /** Construct an empty stack. */
    Stack() {}

    /** Construct a stack from an iterator range. */
    template<class Iterator>
    Stack(const boost::iterator_range<Iterator> &range) {
        for (Iterator iter = range.begin(); iter != range.end(); ++iter)
            items_.pushBack(*iter);
    }

    // FIXME[Robb P. Matzke 2014-08-06]: we need iterators, values(), begin(), end(), etc.

    /** Returns the number of items on the stack. */
    size_t size() const {
        return items_.size();
    }

    /** Determines if the stack is empty.
     *
     *  Returns true if the stack is empty, false if not empty. */
    bool isEmpty() const {
        return items_.isEmpty();
    }

    /** Returns the top item.
     *
     *  Returns a reference to the top item of the stack.  The stack must not be empty.
     *
     *  @{ */
    Value& top() {
        ASSERT_forbid(isEmpty());
        return items_.backValue();
    }
    const Value& top() const {
        ASSERT_forbid(isEmpty());
        return items_.backValue();
    }
    /** @} */

    /** Access an item not at the top of the stack.
     *
     *  Returns a reference to the indicated item.  Item zero is the top of the stack, item one is the next item below the top,
     *  etc.  The stack must not be empty, and @p idx must be less than the size of the stack.
     *
     *  @{ */
    Value& get(size_t idx) {
        ASSERT_require(idx < size());
        return items_[items_.size() - (idx+1)];
    }
    const Value& get(size_t idx) const {
        ASSERT_require(idx < size());
        return items_[items_.size() - (idx+1)];
    }
    Value& operator[](size_t idx) { return get(idx); }
    const Value& operator[](size_t idx) const { return get(idx); }
    /** @} */

    /** Push new item onto stack.
     *
     *  Copies the specified item onto the top of the stack, making the stack one element larger.  The stack itself is returned
     *  so that this method can be chained. */
    Stack& push(const Value &value) {
        items_.pushBack(value);
        return *this;
    }

    /** Pop existing item from stack.
     *
     *  Copies the top item from the stack and returns it, then reduces the size of the stack by one element.  The stack must
     *  not be empty at the time this method is called. */
    Value pop() {
        Value v = top();
        items_.erase(items_.size()-1);
        return v;
    }
};

} // namespace
} // namespace

#endif
