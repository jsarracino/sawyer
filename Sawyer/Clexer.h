// Lexical analyzer for C-like languages
#ifndef Sawyer_Clexer_H
#define Sawyer_Clexer_H

#include <Sawyer/Sawyer.h>

#include <Sawyer/Assert.h>
#include <Sawyer/Buffer.h>
#include <Sawyer/Interval.h>
#include <Sawyer/LineVector.h>

#include <string>
#include <vector>

namespace Sawyer {
namespace Language {
namespace Clexer {

enum TokenType {
    TOK_EOF,                                            // end of file
    TOK_LEFT,                                           // '(', '[', or '{'
    TOK_RIGHT,                                          // ')', ']', or '}'
    TOK_CHAR,                                           // character literal
    TOK_STRING,                                         // string literal
    TOK_NUMBER,                                         // numeric constant, including optional leading sign
    TOK_WORD,                                           // word or symbol name
    TOK_CPP,                                            // preprocessor statement starting with '#'
    TOK_COMMENT,                                        // comment starting with '//' or '/*'
    TOK_OTHER                                           // anything else
};

std::string toString(TokenType);

using Indices = Container::Interval<size_t>;

class Token {
    friend class TokenStream;

    TokenType type_;
    size_t prior_;                                      // start of skipped stuff (whitespace, etc) before begin_
    size_t begin_;                                      // location for first character of token
    size_t end_;                                        // location one past end of token

public:
    Token(): type_(TOK_EOF), prior_(0), begin_(0), end_(0) {} // for std::vector, otherwise not used
    
    Token(TokenType type, size_t prior, size_t begin, size_t end)
        : type_(type), prior_(prior), begin_(begin), end_(end) {
        ASSERT_require(prior <= begin_);
        ASSERT_require(begin <= end);
    }

    TokenType type() const {
        return type_;
    }

    size_t prior() const {
        return prior_;
    }

    size_t begin() const {
        return begin_;
    }

    size_t end() const {
        return end_;
    }

    size_t size() const {
        return end_ - begin_;
    }

    Indices where() const {
        return end_ > begin_ ? Indices::hull(begin_, end_-1) : Indices();
    }

    explicit operator bool() const {
        return type_ != TOK_EOF;
    }

    bool operator!() const {
        return type_ == TOK_EOF;
    }
};

class TokenStream {
private:
    std::string fileName_;                              // name of source file
    Sawyer::Container::LineVector content_;             // contents of source file
    Indices parseRegion_;                               // parse only within this region
    size_t prior_;                                      // one past end of previous token
    size_t at_;                                         // cursor position in buffer
    std::vector<Token> tokens_;                         // token stream filled on demand
    bool skipPreprocessorTokens_;                       // skip over '#' preprocessor directives
    bool skipCommentTokens_;                            // skip over '//' and '/*' comments

public:
    // Parse the contents of a file
    explicit TokenStream(const std::string &fileName)
        : fileName_(fileName), content_(fileName), parseRegion_(Indices::whole()), prior_(0), at_(0),
          skipPreprocessorTokens_(true), skipCommentTokens_(true) {}

    // Parse from buffer
    TokenStream(const std::string &fileName, const Sawyer::Container::Buffer<size_t, char>::Ptr &buffer)
        : fileName_(fileName), content_(buffer), parseRegion_(Indices::whole()), prior_(0), at_(0),
          skipPreprocessorTokens_(true), skipCommentTokens_(true) {}

    // Reparse part of another token stream. Position info, error messages, lines, etc. are from the enclosing token stream.
    TokenStream(TokenStream &super, const Indices &region)
        : fileName_(super.fileName_), content_(super.content_), parseRegion_(region), prior_(region.least()), at_(region.least()),
          skipPreprocessorTokens_(true), skipCommentTokens_(true) {
        ASSERT_require(region);
    }

    const std::string fileName() const { return fileName_; }
    
    bool skipPreprocessorTokens() const { return skipPreprocessorTokens_; }
    void skipPreprocessorTokens(bool b) { skipPreprocessorTokens_ = b; }

    bool skipCommentTokens() const { return skipCommentTokens_; }
    void skipCommentTokens(bool b) { skipCommentTokens_ = b; }

    int getChar(size_t position);

    const Token& operator[](size_t lookahead);

    void consume(size_t n = 1);

    std::string lexeme(const Token &t) const;

    std::string toString(const Token &t) const;

    // Return the line of source in which this token appears, including line termination if present.
    std::string line(const Token &t) const;
    
    bool matches(const Token &token, const char *s2) const;
    bool startsWith(const Token &token, const char *prefix) const;

    void emit(std::ostream &out, const std::string &fileName, const Token &token, const std::string &message) const;

    void emit(std::ostream &out, const std::string &fileName, const Token &begin, const Token &locus, const Token &end,
              const std::string &message) const;

    std::pair<size_t, size_t> location(const Token &token) const;

    const Sawyer::Container::LineVector& content() const {
        return content_;
    }

private:
    void scanString();
    void makeNextToken();
};


} // namespace
} // namespace
} // namespace

#endif
