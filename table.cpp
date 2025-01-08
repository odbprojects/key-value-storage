#include <cassert>
#include "table.h"
#include "exceptions.h"
#include "guard.h"

Table::Table(const std::string& name)
  : m_name(name) {
  pthread_mutex_init(&m_lock, nullptr);
}

Table::~Table() {
  pthread_mutex_destroy(&m_lock);
}

void Table::lock() {
  pthread_mutex_lock(&m_lock);
}

void Table::unlock() {
  pthread_mutex_unlock(&m_lock);
}

bool Table::trylock() {
  return pthread_mutex_trylock(&m_lock) == 0;
}

void Table::set(const std::string& key, const std::string& value) {
  m_pre_data[key] = value;
}

std::string Table::get(const std::string& key) {
  auto it = m_pre_data.find(key);
  if (it != m_pre_data.end()) {
    return it->second;
  }
  it = m_data.find(key);
  if (it != m_data.end()) {
    return it->second;
  }
  throw OperationException("Key not found: " + key);
}

bool Table::has_key(const std::string& key) {
  return m_pre_data.find(key) != m_pre_data.end()|| m_data.find(key) != m_data.end();
}

void Table::commit_changes() {
  for (const auto& kv : m_pre_data) {
    m_data[kv.first] = kv.second;
  }
  m_pre_data.clear();
}

void Table::rollback_changes() {
  m_pre_data.clear();
}
