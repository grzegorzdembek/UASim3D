#pragma once

#include "RedCppLib.hpp"

BEGIN_RED_NAMESPACE;

#if __cpp_exceptions
#include <exception>
class AssertionFailureException : public std::exception
{
    bool has_condition = false;
    bool has_message = false;
    std::string message;
    std::string condition;
    std::string returnmessage;
    public:
    // virtual const char* what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW
    //{
       // if(!has_message && !has_condition)
          //  return "Unspecified exception thrown";
        //
        // return returnmessage.c_str();
    //}
    AssertionFailureException()
    {
        
    }
    AssertionFailureException(std::string condition, std::string message)
    {
        this->message = message;
        this->has_message = true;
        this->condition = condition;
        this->has_condition = true;
        returnmessage = "Condition failed: ()" + condition + ")" + " Message: (" + message + ")";
    }
    AssertionFailureException(std::string condition)
    {
        this->condition = condition;
        this->has_condition = true;
        returnmessage = "Condition failed: ()" + condition + ")";
    }
};

class FailureException : public std::exception
{
    std::string message;
    public:
    //virtual const char* what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW
   // {
      //  if(message.size() == 0)
//return "Unspecified failure";
       // return message.c_str();
  //  }
    FailureException(std::string message = "")
    {
        this->message = message;
    }
};

#endif

inline void _Assertion(bool b=false)
{
    if(b)
        return;
    #if __cpp_exceptions
    throw AssertionFailureException();
    #else
    printf("Unspecified assertion failure");
    int cause_failure = 1/0;
    #endif;
}
inline void _Assertion(bool b, std::string condition)
{
    if (b)
        return;
    #if __cpp_exceptions
    throw AssertionFailureException(condition);
    #else
    printf("Assertion failed for condition: ")
    printf(condition.c_str());
    printf("\n"));
    int cause_failure = 1/0;
    #endif;
}
inline void _Assertion(bool b, std::string condition, std::string message)
{
    if(b)
        return;
    #if __cpp_exceptions
    throw AssertionFailureException(condition, message);
    #else
    printf("Assertion failed for condition: ")
    printf(condition.c_str());
    printf("\n Message: ");
    printf(message.c_str();
    printf("\n"));
    int cause_failure = 1/0;
    #endif;
}
inline void Failure(std::string message = "")
{
    #if __cpp_exceptions
    throw FailureException(message);
    #else
    if(message.size() == 0 )
    {
        printf("Unspecified failure!\n");
    }
    else
    {
        printf("Failure: ");
        printf(message.c_str());
        printf("\n");
    }
    int cause_failure = 1/0;
    #endif;
}

#define GET_NTH_ARG(_1, _2, _3, _4, N, ...) N
#define COUNT_VARARGS(...) GET_NTH_ARG(__VA_ARGS__, 4, 3, 2, 1)

#define PASTE(a,b) PASTE_I(a,b)
#define PASTE_I(a,b) a##b

#define Assert(...) PASTE(_Assert_, COUNT_VARARGS(__VA_ARGS__)(__VA_ARGS__))


#define _Assert_0() _Assertion()
#define _Assert_1(cond) _Assertion((cond), #cond)
#define _Assert_2(cond, msg) _Assertion((cond), #cond, msg)


END_RED_NAMESPACE;