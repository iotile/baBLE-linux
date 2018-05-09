#ifndef BABLE_LINUX_ABSTRACTEXCEPTION_HPP
#define BABLE_LINUX_ABSTRACTEXCEPTION_HPP

#include <string>
#include <exception>
#include "../Log/Loggable.hpp"
#include "constants.hpp"

namespace Exceptions {

  class AbstractException : public std::exception, public Loggable {

  public:
    virtual const Exceptions::Type exception_type() const = 0;
    virtual const std::string exception_name() const = 0;

    const char* what() const noexcept override {
      return stringify().c_str();
    };

  protected:
    explicit AbstractException(const std::string& message) {
      m_message = message;
    };

    std::string m_message;

  };

}

#endif //BABLE_LINUX_ABSTRACTEXCEPTION_HPP