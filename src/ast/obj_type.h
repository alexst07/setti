#ifndef SETI_OBJ_TYPE_H
#define SETI_OBJ_TYPE_H

#include <string>
#include <memory>
#include <unordered_map>
#include <tuple>

#include "run_time_error.h"

namespace setti {
namespace internal {

class EntryPointer {
 public:
  enum class EntryType: uint8_t {
    SYMBOL,
    OBJECT
  };

  EntryType entry_type() const noexcept {
    return type_;
  }

  EntryPointer(const EntryPointer& other): type_(other.type_) {}

 protected:
  EntryPointer(EntryType type): type_(type) {}

 private:
  EntryType type_;
};

class Object: public EntryPointer {
 public:
  enum class ObjectType: uint8_t {
    NIL,
    INT,
    BOOL,
    REAL,
    STRING,
    ARRAY,
    MAP,
    TUPLE,
    CUSTON
  };

  Object(const Object& obj): EntryPointer(obj), type_(obj.type_) {}

  virtual ~Object() {}

  inline ObjectType type() const {
    return type_;
  }

  virtual void Print() = 0;

  virtual std::size_t Hash() const = 0;

  virtual bool operator==(const Object& obj) const = 0;

 private:
  ObjectType type_;

 protected:
  Object(ObjectType type)
      : EntryPointer(EntryPointer::EntryType::OBJECT), type_(type) {}
};

typedef std::shared_ptr<Object> ObjectPtr;

class NullObject: public Object {
 public:
  NullObject(): Object(ObjectType::NIL) {}
  virtual ~NullObject() {}

  std::size_t Hash() const override {
    throw RunTimeError(RunTimeError::ErrorCode::NULL_ACCESS,
                       boost::format("null object has no hash method"));
  }

  bool operator==(const Object& obj) const override {
    if (obj.type() == ObjectType::NIL) {
      return true;
    }

    return false;
  }

  void Print() override {
    std::cout << "NIL";
  }

  inline nullptr_t value() const noexcept { return nullptr; }
};

class IntObject: public Object {
 public:
  IntObject(int value): Object(ObjectType::INT), value_(value) {}
  IntObject(const IntObject& obj): Object(obj), value_(obj.value_) {}
  virtual ~IntObject() {}

  IntObject& operator=(const IntObject& obj) {
    value_ = obj.value_;
    return *this;
  }

  inline int value() const noexcept { return value_; }

  std::size_t Hash() const override {
    std::hash<int> int_hash;
    return int_hash(value_);
  }

  bool operator==(const Object& obj) const override {
    if (obj.type() != ObjectType::INT) {
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                         boost::format("operator == valid only with int"));
    }

    int value = static_cast<const IntObject&>(obj).value_;

    return value_ == value;
  }

  void Print() override {
    std::cout << "INT: " << value_;
  }

 private:
  int value_;
};

class BoolObject: public Object {
 public:
  BoolObject(bool value): Object(ObjectType::BOOL), value_(value) {}
  BoolObject(const BoolObject& obj): Object(obj), value_(obj.value_) {}
  virtual ~BoolObject() {}

  BoolObject& operator=(const BoolObject& obj) {
    value_ = obj.value_;
    return *this;
  }

  inline bool value() const noexcept { return value_; }

  std::size_t Hash() const override {
    std::hash<bool> bool_hash;
    return bool_hash(value_);
  }

  bool operator==(const Object& obj) const override {
    if (obj.type() != ObjectType::BOOL) {
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                         boost::format("operator == valid only with bool"));
    }

    bool value = static_cast<const BoolObject&>(obj).value_;

    return value_ == value;
  }

  void Print() override {
    std::cout << "BOOL: " << value_;
  }

 private:
  bool value_;
};

class RealObject: public Object {
 public:
  RealObject(float value): Object(ObjectType::REAL), value_(value) {}
  RealObject(const RealObject& obj): Object(obj), value_(obj.value_) {}
  virtual ~RealObject() {}

  RealObject& operator=(const RealObject& obj) {
    value_ = obj.value_;
    return *this;
  }

  inline float value() const noexcept { return value_; }

  std::size_t Hash() const override {
    std::hash<float> float_hash;
    return float_hash(value_);
  }

  bool operator==(const Object& obj) const override {
    if (obj.type() != ObjectType::REAL) {
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                         boost::format("operator == valid only with real"));
    }

    float value = static_cast<const RealObject&>(obj).value_;

    return value_ == value;
  }

  void Print() override {
    std::cout << "REAL: " << value_;
  }

 private:
  float value_;
};

class StringObject: public Object {
 public:
  StringObject(std::string&& value)
      : Object(ObjectType::STRING), value_(std::move(value)) {}
  StringObject(const StringObject& obj): Object(obj), value_(obj.value_) {}

  virtual ~StringObject() {}

  StringObject& operator=(const StringObject& obj) {
    value_ = obj.value_;
    return *this;
  }

  inline const std::string& value() const noexcept { return value_; }

  std::size_t Hash() const override {
    std::hash<std::string> str_hash;
    return str_hash(value_);
  }

  bool operator==(const Object& obj) const override {
    if (obj.type() != ObjectType::STRING) {
      throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                         boost::format("operator == valid only with string"));
    }

    std::string value = static_cast<const StringObject&>(obj).value_;

    return value_ == value;
  }

  void Print() override {
    std::cout << "STRING: " << value_;
  }

 private:
  std::string value_;
};
class TupleObject: public Object {
 public:
   TupleObject(std::vector<std::unique_ptr<Object>>&& value)
      : Object(ObjectType::TUPLE), value_(value.size()) {
     for (size_t i = 0; i < value.size(); i++) {
       Object* obj_ptr = value[i].release();
       value_[i] = std::shared_ptr<Object>(obj_ptr);
     }
   }

   TupleObject(std::vector<std::shared_ptr<Object>>&& value)
      : Object(ObjectType::TUPLE), value_(value) {}

   TupleObject(const TupleObject& obj): Object(obj), value_(obj.value_) {}

   virtual ~TupleObject() {}

   inline std::shared_ptr<Object>& ElementRef(size_t i) {
     return value_.at(i);
   }

   inline std::shared_ptr<Object> Element(size_t i) {
     return value_.at(i);
   }

   inline void set(size_t i, std::unique_ptr<Object> obj) {
     Object* obj_ptr = obj.release();
     value_[i] = std::shared_ptr<Object>(obj_ptr);
   }

   std::size_t Hash() const override {
     if (value_.empty()) {
       throw RunTimeError(RunTimeError::ErrorCode::OUT_OF_RANGE,
                          boost::format("hash of empty tuple is not valid"));
     }

     size_t hash = 0;

     // Executes xor operation with hash of each element of tuple
     for (auto& e: value_) {
       hash ^= e->Hash();
     }

     return hash;
   }

   bool operator==(const Object& obj) const override {
     if (obj.type() != ObjectType::TUPLE) {
       throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                          boost::format("operator == valid only with tuple"));
     }

     const TupleObject& tuple_obj = static_cast<const TupleObject&>(obj);

     // If the tuples have different size, they are different
     if (tuple_obj.value_.size() != value_.size()) {
       return false;
     }

     bool r = true;

     // Test each element on tuple
     for (size_t i = 0; i < value_.size(); i++) {
       r = r && (tuple_obj.value_[i] == value_[i]);
     }

     return r;
   }

   void Print() override {
     std::cout << "TUPLE: ( ";
     for (const auto& e: value_) {
       e->Print();
       std::cout << " ";
     }
     std::cout << ")";
   }

 private:
  std::vector<std::shared_ptr<Object>> value_;
};

class ArrayObject: public Object {
 public:
   ArrayObject(std::vector<std::unique_ptr<Object>>&& value)
      : Object(ObjectType::ARRAY), value_(value.size()) {
     for (size_t i = 0; i < value.size(); i++) {
       Object* obj_ptr = value[i].release();
       value_[i] = std::shared_ptr<Object>(obj_ptr);
     }
   }

   ArrayObject(std::vector<std::shared_ptr<Object>>&& value)
      : Object(ObjectType::ARRAY), value_(value) {}

   ArrayObject(const ArrayObject& obj): Object(obj), value_(obj.value_) {}

   virtual ~ArrayObject() {}

   inline Object* at(size_t i) {
     return value_.at(i).get();
   }

   inline std::shared_ptr<Object>& ElementRef(size_t i) {
     return value_.at(i);
   }

   inline std::shared_ptr<Object> Element(size_t i) {
     return value_.at(i);
   }

   inline void set(size_t i, std::unique_ptr<Object> obj) {
     Object* obj_ptr = obj.release();
     value_[i] = std::shared_ptr<Object>(obj_ptr);
   }

   std::size_t Hash() const override {
     if (value_.empty()) {
       throw RunTimeError(RunTimeError::ErrorCode::OUT_OF_RANGE,
                          boost::format("hash of empty array is not valid"));
     }

     size_t hash = 0;

     // Executes xor operation with hash of each element of array
     for (auto& e: value_) {
       hash ^= e->Hash();
     }

     return hash;
   }

   bool operator==(const Object& obj) const override {
     if (obj.type() != ObjectType::ARRAY) {
       throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                          boost::format("operator == valid only with array"));
     }

     const ArrayObject& array_obj = static_cast<const ArrayObject&>(obj);

     // If the tuples have different size, they are different
     if (array_obj.value_.size() != value_.size()) {
       return false;
     }

     bool r = true;

     // Test each element on tuple
     for (size_t i = 0; i < value_.size(); i++) {
       r = r && (array_obj.value_[i] == value_[i]);
     }

     return r;
   }

   void Print() override {
     std::cout << "ARRAY: [ ";
     for (const auto& e: value_) {
       e->Print();
       std::cout << " ";
     }
     std::cout << "]";
   }

 private:
  std::vector<std::shared_ptr<Object>> value_;
};

}
}

#endif  // SETI_OBJ_TYPE_H

