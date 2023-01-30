#ifndef PROPERTYHELPER_H
#define PROPERTYHELPER_H
#include <QObject>
// See Gist Comment for description, usage, warnings and license information
#define AUTO_PROPERTY(TYPE, NAME, SET_NAME)                                 \
  Q_PROPERTY(TYPE NAME READ NAME WRITE set##SET_NAME NOTIFY NAME##Changed) \
 public:                                                          \
  TYPE NAME() const { return m_##NAME; }                          \
  void set##SET_NAME(const TYPE& value) {                                  \
    if (m_##NAME == value)                                        \
      return;                                                     \
    m_##NAME = value;                                             \
    emit NAME##Changed(value);                                    \
  }                                                               \
  Q_SIGNAL void NAME##Changed(TYPE value);                        \
                                                                  \
 private:                                                         \
  TYPE m_##NAME;

#define READONLY_PROPERTY(TYPE, NAME)      \
  Q_PROPERTY(TYPE NAME READ NAME CONSTANT) \
 public:                                   \
  TYPE NAME() const { return m_##NAME; }   \
                                           \
 private:                                  \
  TYPE m_##NAME;

#define READ_PROPERTY(TYPE, NAME)                      \
  Q_PROPERTY(TYPE NAME READ NAME NOTIFY NAME##Changed) \
 public:                                               \
  TYPE NAME() const { return m_##NAME; }               \
  Q_SIGNAL void NAME##Changed(TYPE value);             \
                                                       \
 private:                                              \
  TYPE m_##NAME;
#endif  // PROPERTYHELPER_H
