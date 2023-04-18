// Test for Rose::Tree
#include <Sawyer/Tree.h>

#include <Sawyer/Optional.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// A few tree vertex types
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------------------------------------------------------
// This part is typical of a BasicTypes.h header
class Expression;
using ExpressionPtr = std::shared_ptr<Expression>;
using ExpressionList = Sawyer::Tree::List<Expression, Expression>;
using ExpressionListPtr = std::shared_ptr<ExpressionList>;
class BinaryExpression;
using BinaryExpressionPtr = std::shared_ptr<BinaryExpression>;
class Recursive;
using RecursivePtr = std::shared_ptr<Recursive>;
class BinaryTree;
using BinaryTreePtr = std::shared_ptr<BinaryTree>;
class Multi;
using MultiPtr = std::shared_ptr<Multi>;

//------------------------------------------------------------------------------------------------------------------------------
// This part is typical of a header, although the implementations are usually elsewhere

// User base class for expression trees
class Expression: public Sawyer::Tree::Vertex<Expression> {
public:
    using Ptr = ExpressionPtr;
protected:
    Expression() {}
};

// Expression vertex that has a left- and right-hand-side children, plus a list child that is allocated by the constructor. This is
// more or less the ROSE way of making an AST, where a list of children is always contained in a separate node whose only purpose is
// to store the list.
class BinaryExpression: public Expression {
public:
    using Ptr = BinaryExpressionPtr;
    Edge<Expression> lhs;
    Edge<Expression> rhs;
    Edge<ExpressionList> list;

protected:
    BinaryExpression()
        : lhs(*this), rhs(*this), list(*this, ExpressionList::instance()) {}

public:
    static Ptr instance() {
        return Ptr(new BinaryExpression);
    }
};

// Another expression that has 'a', 'b', and 'c' edges where 'a' and 'c' are normal scalar edges but 'b' is a multi-edge that points
// to a variable number of children.
class Multi: public Expression {
public:
    using Ptr = MultiPtr;
    Edge<Expression> a;
    EdgeVector<Expression> b;
    Edge<Expression> c;

protected:
    Multi()
        : a(*this), b(*this), c(*this) {}

public:
    static Ptr instance() {
        return Ptr(new Multi);
    }
};

// A binary tree has left and right pointers and no other pointers
class BinaryTree: public Sawyer::Tree::Vertex<BinaryTree> {
public:
    using Ptr = BinaryTreePtr;
    Edge<BinaryTree> left;
    Edge<BinaryTree> right;

protected:
    BinaryTree()
        : left(*this), right(*this) {}

public:
    static Ptr instance() {
        return Ptr(new BinaryTree);
    }

    static Ptr instance(const Ptr &left, const Ptr &right) {
        auto self = instance();
        self->left = left;
        self->right = right;
        return self;
    }
};

// A vertex whose children are the same type.
class Recursive: public Sawyer::Tree::Vertex<Recursive> {
public:
    using Ptr = RecursivePtr;
    Edge<Recursive> child;

protected:
    Recursive()
        : child(*this) {}

public:
    static Ptr instance() {
        return Ptr(new Recursive);
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tests
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// vertex pointers are initialized to null
static void test01() {
    BinaryExpressionPtr e;
    ASSERT_always_require(e == nullptr);
}

// instantion can use `auto` and the type is mentioned on the rhs
static void test02() {
    auto e = BinaryExpression::instance();
    ASSERT_always_require(e != nullptr);
}

// child edges are initialized to null
static void test03() {
    auto e = BinaryExpression::instance();
    ASSERT_always_require(e->lhs == nullptr);
    ASSERT_always_require(e->rhs == nullptr);
}

// initialized child edges are initialized
static void test04() {
    auto e = BinaryExpression::instance();
    ASSERT_always_require(e != nullptr);
}

// you can get a pointer from a vertex object
static void test05() {
    auto e = BinaryExpression::instance();
    auto f = [](BinaryExpression &vertex) {
        return vertex.pointer();
    }(*e);
    ASSERT_always_require(e == f);
}

// parent pointers are initialized to null
static void test06() {
    auto e = BinaryExpression::instance();
    ASSERT_always_require(e->parent == nullptr);
}

// inserting a child changes its parent pointer
static void test07() {
    auto parent = BinaryExpression::instance();
    auto child = BinaryExpression::instance();

    parent->lhs = child;
    ASSERT_always_require(parent->lhs == child);
    ASSERT_always_require(child->parent == parent);
}

// erasing a child resets its parent pointer
static void test08() {
    auto parent = BinaryExpression::instance();
    auto child = BinaryExpression::instance();
    parent->lhs = child;

    parent->lhs = nullptr;
    ASSERT_always_require(parent->lhs == nullptr);
    ASSERT_always_require(child->parent == nullptr);
}

// inserting a different child changes both children's parent pointers
static void test09() {
    auto parent = BinaryExpression::instance();
    auto child = BinaryExpression::instance();
    parent->lhs = child;

    auto child2 = BinaryExpression::instance();
    parent->lhs = child2;
    ASSERT_always_require(parent->lhs == child2);
    ASSERT_always_require(child2->parent == parent);
    ASSERT_always_require(child->parent == nullptr);
}

// reassigning a child is a no-op
static void test10() {
    auto parent = BinaryExpression::instance();
    auto child = BinaryExpression::instance();
    parent->lhs = child;

    parent->lhs = child;
    ASSERT_always_require(parent->lhs == child);
    ASSERT_always_require(child->parent == parent);
}

// inserting a child in two different places is an error with no side effect
static void test11() {
    auto parent = BinaryExpression::instance();
    auto child = BinaryExpression::instance();
    parent->lhs = child;

    try {
        parent->rhs = child;
        ASSERT_always_not_reachable("data structure is no longer a tree");
    } catch (const Expression::InsertionError &error) {
        ASSERT_always_require(error.vertex == child);
        ASSERT_always_require(parent->lhs == child);
        ASSERT_always_require(child->parent == parent);
        ASSERT_always_require(parent->rhs == nullptr);
    }
}

// inserting a child into two different trees is an error with no side effect
static void test12() {
    auto parent = BinaryExpression::instance();
    auto child = BinaryExpression::instance();
    parent->lhs = child;

    auto parent2 = BinaryExpression::instance();
    try {
        parent2->lhs = child;
        ASSERT_always_not_reachable("data structure is no longer a tree");
    } catch (Expression::InsertionError &error) {
        ASSERT_always_require(error.vertex == child);
        ASSERT_always_require(parent->lhs == child);
        ASSERT_always_require(child->parent == parent);
        ASSERT_always_require(parent2->lhs == nullptr);
    }
}

// inserting a child as its own parent is an error with no side effect
static void test13() {
    auto r = Recursive::instance();

#ifdef NDEBUG
    // Allowed in production configurations since it is not a constant time check
    r->child = r;
#else
    try {
        r->child = r;
        ASSERT_always_not_reachable("data structure is no longer a tree");
    } catch (const Recursive::CycleError &error) {
        ASSERT_always_require(error.vertex == r);
        ASSERT_always_require(r->child == nullptr);
        ASSERT_always_require(r->parent == nullptr);
    }
#endif
}

// inserting a child as its own descendant is an error with no side effect
static void test14() {
    auto r1 = Recursive::instance();
    auto r2 = Recursive::instance();
    auto r3 = Recursive::instance();
    r1->child = r2;
    r2->child = r3;

#ifdef NDEBUG
    // Allowed in production configurations since it is not a constant time check
    r3->child = r1;
#else
    try {
        r3->child = r1;
        ASSERT_always_not_reachable("data structure is no longer a tree");
    } catch (const Recursive::CycleError &error) {
        ASSERT_always_require(error.vertex == r1);
        ASSERT_always_require(r1->child == r2);
        ASSERT_always_require(r2->child == r3);
        ASSERT_always_require(r3->child == nullptr);
        ASSERT_always_require(r3->parent == r2);
        ASSERT_always_require(r2->parent == r1);
        ASSERT_always_require(r1->parent == nullptr);
    }
#endif
}

// lists are initialized to be empty
static void test15() {
    auto s = ExpressionList::instance();
    ASSERT_always_require(s->empty());
    ASSERT_always_require(s->size() == 0);
}

// null pointers can be pushed and popped
static void test16() {
    auto s = ExpressionList::instance();

    s->push_back(nullptr);
    ASSERT_always_require(!s->empty());
    ASSERT_always_require(s->size() == 1);

    s->push_back(nullptr);
    ASSERT_always_require(s->size() == 2);
    ASSERT_always_require(s->at(0) == nullptr);

    s->pop_back();
    ASSERT_always_require(s->size() == 1);
    ASSERT_always_require(s->at(0) == nullptr);

    s->pop_back();
    ASSERT_always_require(s->size() == 0);
    ASSERT_always_require(s->empty());
}

// pushing a child changes its parent pointer
static void test17() {
    auto parent = ExpressionList::instance();
    auto child = BinaryExpression::instance();

    parent->push_back(child);
    ASSERT_always_require(parent->size() == 1);
    ASSERT_always_require(parent->at(0) == child);
    ASSERT_always_require(child->parent == parent);
}

// popping a child clears its parent pointer
static void test18() {
    auto parent = ExpressionList::instance();
    auto child = BinaryExpression::instance();
    parent->push_back(child);

    parent->pop_back();
    ASSERT_always_require(child->parent == nullptr);
}

// assigning a child to a list changes its parent pointer
static void test19() {
    auto parent = ExpressionList::instance();
    parent->push_back(nullptr);

    auto child = BinaryExpression::instance();
    parent->at(0) = child;
    ASSERT_always_require(parent->at(0) == child);
    ASSERT_always_require(child->parent == parent);
}

// overwriting a child in a list changes its parent pointer
static void test20() {
    auto parent = ExpressionList::instance();
    auto child = BinaryExpression::instance();
    parent->push_back(child);

    auto child2 = BinaryExpression::instance();
    parent->at(0) = child2;
    ASSERT_always_require(parent->at(0) == child2);
    ASSERT_always_require(child2->parent == parent);
    ASSERT_always_require(child->parent == nullptr);
}

// reassigning a child to a list is a no-op
static void test21() {
    auto parent = ExpressionList::instance();
    auto child = BinaryExpression::instance();
    parent->push_back(child);

    parent->at(0) = child;
    ASSERT_always_require(parent->at(0) == child);
    ASSERT_always_require(child->parent == parent);
}

// inserting a child twice is an error with no side effect
static void test22() {
    auto parent = ExpressionList::instance();
    auto child = BinaryExpression::instance();
    parent->push_back(child);

    try {
        parent->push_back(child);
        ASSERT_always_not_reachable("data structure is no longer a tree");
    } catch (const Expression::InsertionError &error) {
        ASSERT_always_require(error.vertex == child);
        ASSERT_always_require(parent->size() == 1);
        ASSERT_always_require(parent->at(0) == child);
        ASSERT_always_require(child->parent == parent);
    }
}

// assigning a child to a second element is an error with no side effect
static void test23() {
    auto parent = ExpressionList::instance();
    auto child = BinaryExpression::instance();
    parent->push_back(child);
    parent->push_back(nullptr);

    try {
        parent->at(1) = child;
        ASSERT_always_not_reachable("data structure is no longer a tree");
    } catch (const Expression::InsertionError &error) {
        ASSERT_always_require(error.vertex == child);
        ASSERT_always_require(parent->size() == 2);
        ASSERT_always_require(parent->at(0) == child);
        ASSERT_always_require(parent->at(1) == nullptr);
        ASSERT_always_require(child->parent == parent);
    }
}

// replacing a child with null resets its parent pointer
static void test24() {
    auto parent = ExpressionList::instance();
    auto child = BinaryExpression::instance();
    parent->push_back(child);

    parent->at(0) = nullptr;
    ASSERT_always_require(parent->at(0) == nullptr);
    ASSERT_always_require(child->parent == nullptr);
}

// the array operator works
static void test25() {
    auto parent = ExpressionList::instance();
    auto c1 = BinaryExpression::instance();
    auto c2 = BinaryExpression::instance();
    parent->push_back(c1);
    parent->push_back(nullptr);
    parent->push_back(c2);

    ASSERT_always_require((*parent)[0] == c1);
    ASSERT_always_require((*parent)[1] == nullptr);
    ASSERT_always_require((*parent)[2] == c2);

    (*parent)[2] = nullptr;
    ASSERT_always_require((*parent)[2] == nullptr);
    ASSERT_always_require(c2->parent == nullptr);
}

// list iteration works
static void test26() {
    auto parent = ExpressionList::instance();
    auto c1 = BinaryExpression::instance();
    auto c2 = BinaryExpression::instance();
    parent->push_back(c1);
    parent->push_back(nullptr);
    parent->push_back(c2);

    auto iter = parent->begin();
    ASSERT_always_require(iter != parent->end());
    ASSERT_always_require(*iter == c1);
    ASSERT_always_require(iter[0] == c1);
    ASSERT_always_require(iter[1] == nullptr);
    ASSERT_always_require(iter[2] == c2);

    auto iter2 = ++iter;
    ASSERT_always_require(iter2 == iter);
    ASSERT_always_require(!(iter2 < iter));
    ASSERT_always_require(iter2 - iter == 0);
    ASSERT_always_require(iter2 + 0 == iter);
    ASSERT_always_require(iter != parent->end());
    ASSERT_always_require(*iter == nullptr);

    const auto iter3 = iter++;
    ASSERT_always_require(iter3 != iter);
    ASSERT_always_require(iter3 < iter);
    ASSERT_always_require(iter3 - iter == 1);
    ASSERT_always_require(iter3 + 1 == iter);
    ASSERT_always_require(iter != parent->end());
    ASSERT_always_require(*iter == c2);
    ASSERT_always_require(iter3[1] == c2);

    ++iter;
    ASSERT_always_require(iter == parent->end());

    --iter;
    ASSERT_always_require(*iter == c2);

    iter -= 2;
    ASSERT_always_require(*iter == c1);

    iter += 3;
    ASSERT_always_require(iter == parent->end());
}

// forward traversals visit children
static void test27() {
    auto parent = BinaryTree::instance();
    auto child1 = BinaryTree::instance();
    auto child2 = BinaryTree::instance();
    parent->left = child1;
    parent->right = child2;

    std::vector<BinaryTree::Ptr> answer;
    answer.push_back(child2);
    answer.push_back(child1);
    answer.push_back(parent);

    parent->traverse<BinaryTree>([&answer](const BinaryTree::Ptr &node, BinaryTree::TraversalEvent event) {
        if (BinaryTree::TraversalEvent::ENTER == event) {
            ASSERT_always_forbid(answer.empty());
            ASSERT_always_require(answer.back() == node);
            answer.pop_back();
        }
        return false;
    });
}

// a multi edge has zero initial children
static void test28() {
    auto parent = Multi::instance();
    ASSERT_always_require(parent->b.empty());
    ASSERT_always_require(parent->b.size() == 0);
    ASSERT_always_require(parent->nChildren() == 2);    // a=1, b=0, c=1
}

// pushing to a multi-edge increments the number of children
static void test29() {
    auto parent = Multi::instance();

    parent->b.push_back(nullptr);
    ASSERT_always_require(!parent->b.empty());
    ASSERT_always_require(parent->b.size() == 1);
    ASSERT_always_require(parent->nChildren() == 3);    // a=1, b=1, c=1

    parent->b.push_back(nullptr);
    ASSERT_always_require(parent->b.size() == 2);
    ASSERT_always_require(parent->nChildren() == 4);    // a=1, b=2, c=1
}

// pushing a child changes the child's parent pointer
static void test30() {
    auto parent = Multi::instance();
    auto child = Multi::instance();

    parent->b.push_back(child);
    ASSERT_always_require(parent->b.size() == 1);
    ASSERT_always_require(parent->b[0] == child);
    ASSERT_always_require(child->parent == parent);
}

// popping a child resets its parent pointer
static void test31() {
    auto parent = Multi::instance();
    auto child = Multi::instance();
    parent->b.push_back(child);

    parent->b.pop_back();
    ASSERT_always_require(parent->b.empty());
    ASSERT_always_require(parent->b.size() == 0);
    ASSERT_always_require(child->parent == nullptr);
}

// re-assigning the child is a no-op
static void test32() {
    auto parent = Multi::instance();
    auto child = Multi::instance();
    parent->b.push_back(child);

    parent->b[0] = child;
    ASSERT_always_require(parent->b.size() == 1);
    ASSERT_always_require(parent->b[0] == child);
    ASSERT_always_require(child->parent == parent);
}

// assigning null resets the child's parent pointer
static void test33() {
    auto parent = Multi::instance();
    auto child = Multi::instance();
    parent->b.push_back(child);

    parent->b[0] = nullptr;
    ASSERT_always_require(parent->b.at(0) == nullptr);
    ASSERT_always_require(parent->b[0] == nullptr);
    ASSERT_always_require(child->parent == nullptr);
}

// assigning a different child changes both children's parent pointers
static void test34() {
    auto parent = Multi::instance();
    auto child1 = Multi::instance();
    auto child2 = Multi::instance();
    parent->b.push_back(child1);

    parent->b[0] = child2;
    ASSERT_always_require(parent->b[0] == child2);
    ASSERT_always_require(child2->parent == parent);
    ASSERT_always_require(child1->parent == nullptr);
}

// inserting a child twice is an error with no side effect
static void test35() {
    auto parent = Multi::instance();
    auto child = Multi::instance();
    parent->b.push_back(child);

    try {
        parent->b.push_back(child);
        ASSERT_not_reachable("data structure is no longer a tree");
    } catch (const Multi::InsertionError &error) {
        ASSERT_always_require(error.vertex == child);
        ASSERT_always_require(parent->b.size() == 1);
        ASSERT_always_require(parent->b[0] == child);
        ASSERT_always_require(child->parent == parent);
    }
}

// assigning the same child again is an error with no side effect
static void test36() {
    auto parent = Multi::instance();
    auto child = Multi::instance();
    parent->b.push_back(child);
    parent->b.push_back(nullptr);

    try {
        parent->b[1] = child;
        ASSERT_always_not_reachable("data structure is no longer a tree");
    } catch (const Expression::InsertionError &error) {
        ASSERT_always_require(error.vertex == child);
        ASSERT_always_require(parent->b.size() == 2);
        ASSERT_always_require(parent->b[0] == child);
        ASSERT_always_require(parent->b[1] == nullptr);
        ASSERT_always_require(child->parent == parent);
    }
}

// multi edge iteration works
static void test37() {
    auto parent = Multi::instance();
    auto child1 = Multi::instance();
    auto child2 = Multi::instance();
    parent->b.push_back(child1);
    parent->b.push_back(nullptr);
    parent->b.push_back(child2);

    auto iter = parent->b.begin();
    ASSERT_always_require(iter != parent->b.end());
    ASSERT_always_require(*iter == child1);
    ASSERT_always_require(iter[0] == child1);
    ASSERT_always_require(iter[1] == nullptr);

    auto iter2 = ++iter;
    ASSERT_always_require(iter2 == iter);
    ASSERT_always_require(!(iter2 < iter));
    ASSERT_always_require(iter2 - iter == 0);
    ASSERT_always_require(iter2 + 0 == iter);
    ASSERT_always_require(iter != parent->b.end());
    ASSERT_always_require(*iter == nullptr);

    const auto iter3 = iter++;
    ASSERT_always_require(iter3 != iter);
    ASSERT_always_require(iter3 < iter);
    ASSERT_always_require(iter3 - iter == 1);
    ASSERT_always_require(iter != parent->b.end());
    ASSERT_always_require(*iter == child2);
    ASSERT_always_require(iter3[1] == child2);

    ++iter;
    ASSERT_always_require(iter == parent->b.end());

    --iter;
    ASSERT_always_require(*iter == child2);

    iter -= 2;
    ASSERT_always_require(*iter == child1);

    iter += 3;
    ASSERT_always_require(iter == parent->b.end());
}

// child can be changed through operations
static void test38() {
    auto parent = Multi::instance();
    auto child1 = Multi::instance();
    auto child2 = Multi::instance();
    parent->b.push_back(child1);

    parent->b[0] = child2;
    ASSERT_require(parent->b[0] == child2);
    ASSERT_require(child1->parent == nullptr);
    ASSERT_require(child2->parent == parent);

    parent->b.at(0) = child1;
    ASSERT_require(parent->b[0] == child1);
    ASSERT_require(child1->parent == parent);
    ASSERT_require(child2->parent == nullptr);

    parent->b.front() = child2;
    ASSERT_require(parent->b[0] == child2);
    ASSERT_require(child1->parent == nullptr);
    ASSERT_require(child2->parent == parent);

    parent->b.back() = child1;
    ASSERT_require(parent->b[0] == child1);
    ASSERT_require(child1->parent == parent);
    ASSERT_require(child2->parent == nullptr);

    (*parent->b.begin()) = child2;
    ASSERT_require(parent->b[0] == child2);
    ASSERT_require(child1->parent == nullptr);
    ASSERT_require(child2->parent == parent);
}

// multi-child can be used in range-for
static void test39() {
    auto parent = Multi::instance();
    auto child1 = Multi::instance();
    auto child2 = Multi::instance();
    parent->b.push_back(child1);
    parent->b.push_back(nullptr);
    parent->b.push_back(child2);

    int i = 0;
    for (const auto &c: parent->b) {
        switch (i++) {
            case 0:
                ASSERT_always_require(c == child1);
                break;
            case 1:
                ASSERT_always_require(c == nullptr);
                break;
            case 2:
                ASSERT_always_require(c == child2);
                break;
            default:
                ASSERT_not_reachable("too many iterations");
        }
    }
}

// multi-child can be changed through range-for
static void test40() {
    auto parent = Multi::instance();
    auto child1 = Multi::instance();
    auto child2 = Multi::instance();
    parent->b.push_back(child1);
    parent->b.push_back(nullptr);
    parent->b.push_back(child2);

    for (auto &c: parent->b) {
        if (c) {
            c = nullptr;
        } else {
            c = child1;
        }
    }

    ASSERT_always_require(parent->b.size() == 3);
    ASSERT_always_require(parent->b[0] == nullptr);
    ASSERT_always_require(parent->b[1] == child1);
    ASSERT_always_require(parent->b[2] == nullptr);
    ASSERT_always_require(child1->parent == parent);
    ASSERT_always_require(child2->parent == nullptr);
}

// forward traversals visit only non-null children
static void test41() {
    auto parent = Multi::instance();
    auto child1 = Multi::instance();
    auto child2 = Multi::instance();
    parent->b.push_back(child1);
    parent->b.push_back(nullptr);
    parent->b.push_back(child2);

    size_t nEnter = 0, nLeave = 0;
    parent->traverse<Multi>([&nEnter, &nLeave](const Multi::Ptr &vertex, Multi::TraversalEvent event) {
        ASSERT_always_not_null(vertex);
        switch (event) {
            case Multi::TraversalEvent::ENTER:
                ++nEnter;
                break;
            case Multi::TraversalEvent::LEAVE:
                ++nLeave;
                break;
            default:
                ASSERT_not_reachable("invalid event");
        }
        return false;
    });

    ASSERT_always_require2(3 == nEnter, nEnter);
    ASSERT_always_require2(3 == nLeave, nLeave);
}

// forward traversals can be restricted to particular types
static void test42() {
    auto parent = Multi::instance();
    auto child1 = Multi::instance();
    auto child2 = BinaryExpression::instance();
    parent->a = child1;
    parent->c = child2;

    size_t n = 0;
    parent->traverse<BinaryExpression>([&n](const ExpressionPtr&, Expression::TraversalEvent) {
        ++n;
        return false;
    });
    ASSERT_always_require2(2 == n, n);                  // enter and leave for 1 vertex

    n = 0;
    parent->traverse<Multi>([&n](const ExpressionPtr&, Expression::TraversalEvent) {
        ++n;
        return false;
    });
    ASSERT_always_require2(4 == n, n);                  // enter and leave for 2 vertices

    n = 0;
    parent->traverse<Expression>([&n](const ExpressionPtr&, Expression::TraversalEvent) {
        ++n;
        return false;
    });
    ASSERT_always_require2(8 == n, n);                  // enter and leave for 4 vertices (including child2.list)
}

// forward traversals can exit early
static void test43() {
    auto parent = Multi::instance();
    auto a = Multi::instance();
    auto b1 = Multi::instance();
    auto b2 = Multi::instance();
    auto c = Multi::instance();
    parent->a = a;
    parent->b.push_back(b1);
    parent->b.push_back(b2);
    parent->c = c;

    size_t n = 0;
    bool result = parent->traverse<Multi>([&n, &b2](const MultiPtr &vertex, Expression::TraversalEvent event) {
        if (Expression::TraversalEvent::ENTER == event)
            ++n;
        return vertex == b2;
    });
    ASSERT_always_require(result);
    ASSERT_always_require2(4 == n, n);
}

// forward traversals don't have to return `bool` to exit early
static void test44() {
    auto parent = Multi::instance();
    auto a = Multi::instance();
    auto b1 = Multi::instance();
    auto b2 = Multi::instance();
    auto c = Multi::instance();
    parent->a = a;
    parent->b.push_back(b1);
    parent->b.push_back(b2);
    parent->c = c;

    MultiPtr result = parent->traverse<Multi>([&b2](const MultiPtr &vertex, Expression::TraversalEvent) {
        if (vertex == b2) {
            return b2;
        } else {
            return MultiPtr();
        }
    });
    ASSERT_always_require(result == b2);
}

// traversals can return user-defined types
static void test45() {
    auto parent = Multi::instance();
    auto a = Multi::instance();
    auto b1 = Multi::instance();
    auto b2 = Multi::instance();
    auto c = Multi::instance();
    parent->a = a;
    parent->b.push_back(b1);
    parent->b.push_back(b2);
    parent->c = c;

    class MyType {
    public:
        int x = 0;

        // This defines when to terminate
        explicit operator bool() const {
            return x == 911;
        }
    };

    MyType result = parent->traverse<Multi>([&b2](const MultiPtr &vertex, Expression::TraversalEvent) {
        MyType retval;
        retval.x = vertex == b2 ? 911 : 42;
        return retval;
    });
    ASSERT_always_require2(result.x == 911, result.x);
}

// Sawyer::Optional can be used to easily return a value to short circuit.
static void test46() {
    auto parent = Multi::instance();
    auto a = Multi::instance();
    auto b1 = Multi::instance();
    auto b2 = Multi::instance();
    auto c = Multi::instance();
    parent->a = a;
    parent->b.push_back(b1);
    parent->b.push_back(b2);
    parent->c = c;

    const int x = parent->traverse<Multi>([&b2](const MultiPtr &vertex, Expression::TraversalEvent) {
        if (vertex == b2) {
            return Sawyer::Optional<int>(911);
        } else {
            return Sawyer::Optional<int>();
        }
    }).orElse(42);

    ASSERT_always_require2(x == 911, x);
}

// A return value can be thrown, although this is not as clean and can be slower, especially if you want the result to be const
static void test47() {
    auto parent = Multi::instance();
    auto a = Multi::instance();
    auto b1 = Multi::instance();
    auto b2 = Multi::instance();
    auto c = Multi::instance();
    parent->a = a;
    parent->b.push_back(b1);
    parent->b.push_back(b2);
    parent->c = c;

    const int x = [&b2, &parent]() {
        try {
            parent->traverse<Multi>([&b2](const MultiPtr &vertex, Expression::TraversalEvent) {
                if (vertex == b2) {
                    throw 911;
                } else {
                    return false;
                }
            });
            return 42;
        } catch (int result) {
            return result;
        }
    }();

    ASSERT_always_require2(x == 911, x);
}

int main() {
    test01();
    test02();
    test03();
    test04();
    test05();
    test06();
    test07();
    test08();
    test09();
    test10();
    test11();
    test12();
    test13();
    test14();
    test15();
    test16();
    test17();
    test18();
    test19();
    test20();
    test21();
    test22();
    test23();
    test24();
    test25();
    test26();
    test27();
    test28();
    test29();
    test30();
    test31();
    test32();
    test33();
    test34();
    test35();
    test36();
    test37();
    test38();
    test39();
    test40();
    test41();
    test42();
    test43();
    test44();
    test45();
    test46();
    test47();
}
