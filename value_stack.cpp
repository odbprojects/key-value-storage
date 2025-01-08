#include "value_stack.h"
#include "exceptions.h"

ValueStack::ValueStack()
{
}

ValueStack::~ValueStack()
{
}

bool ValueStack::is_empty() const
{
  return m_stack.empty();
}

void ValueStack::push(const std::string &value)
{
  m_stack.push_back(value);
}

std::string ValueStack::get_top() const
{
  if (is_empty()) {
    throw OperationException("Stack is empty");
  }
  return m_stack.back();
}

void ValueStack::pop()
{
  if (is_empty()) {
    throw OperationException("Stack is empty");
  }
  m_stack.pop_back();
}
