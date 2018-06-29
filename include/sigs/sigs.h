/***************************************************************************************************
sigs

Simple thread-safe signal/slot C++14 library.

The MIT License (MIT)

Copyright (c) 2015 Morten Kristensen, msk AT nullpointer DOT dk

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
***************************************************************************************************/

#ifndef SIGS_SIGNAL_SLOT_H
#define SIGS_SIGNAL_SLOT_H

#include <cstddef>
#include <functional>
#include <initializer_list>
#include <memory>
#include <mutex>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// The following is used for making variadic type lists for binding member functions and their
// parameters.
namespace sigs {

template <std::size_t... Ns>
class Seq {
};

template <std::size_t N, std::size_t... Ns>
class MakeSeq : public MakeSeq<N - 1, N - 1, Ns...> {
};

template <std::size_t... Ns>
class MakeSeq<0, Ns...> : public Seq<Ns...> {
};

template <std::size_t>
class Placeholder {
};

} // namespace sigs

// std::bind uses std::is_placeholder to detect placeholders for unbounded arguments, so it must be
// overridden to accept the custom sigs::Placeholder type.
namespace std {

template <size_t N>
class is_placeholder<sigs::Placeholder<N>> : public integral_constant<std::size_t, N + 1> {
};

} // namespace std

namespace sigs {

/// When a member function has muliple overloads and you need to use just one of them.
/** Example:
    signal.connect(&instance, sigs::Use<int, float>::overloadOf(&ThClass::func));
    */
template <typename... Args>
struct Use {
  template <typename Cls, typename Ret>
  static auto overloadOf(Ret (Cls::*MembFunc)(Args...))
  {
    return MembFunc;
  }
};

class ConnectionBase {
  template <typename>
  friend class Signal;

public:
  void disconnect()
  {
    if (deleter) deleter();
  }

private:
  std::function<void()> deleter;
};

using Connection = std::shared_ptr<ConnectionBase>;

namespace {

/// VoidableFunction is used internally to generate a function type depending on whether the return
/// type of the signal is non-void.
template <typename T>
class VoidableFunction {
public:
  using func = std::function<void(T)>;
};

/// Specialization for void return types.
template <>
class VoidableFunction<void> {
public:
  using func = std::function<void()>;
};

} // namespace

template <typename>
class Signal;

template <typename Ret, typename... Args>
class Signal<Ret(Args...)> {
  using Slot = std::function<Ret(Args...)>;

  class Entry {
  public:
    Entry(const Slot &slot, Connection conn) : slot_(slot), conn_(conn), signal_(nullptr)
    {
    }

    Entry(Slot &&slot, Connection conn) : slot_(std::move(slot)), conn_(conn), signal_(nullptr)
    {
    }

    Entry(Signal *signal, Connection conn) : conn_(conn), signal_(signal)
    {
    }

    const Slot &slot() const
    {
      return slot_;
    }

    Signal *signal() const
    {
      return signal_;
    }

    Connection conn() const
    {
      return conn_;
    }

  private:
    Slot slot_;
    Connection conn_;
    Signal *signal_;
  };

  using Cont = std::vector<Entry>;
  using Lock = std::lock_guard<std::mutex>;

public:
  using ReturnType = Ret;
  using SlotType = Slot;

  Signal()
  {
  }

  virtual ~Signal()
  {
    Lock lock(entriesMutex);
    for (auto &entry : entries) {
      auto conn = entry.conn();
      if (conn) {
        conn->deleter = nullptr;
      }
    }
  }

  Signal(const Signal &rhs)
  {
    Lock lock1(entriesMutex);
    Lock lock2(const_cast<Signal &>(rhs).entriesMutex);
    entries = rhs.entries;
  }

  Signal &operator=(const Signal &rhs)
  {
    Lock lock1(entriesMutex);
    Lock lock2(const_cast<Signal &>(rhs).entriesMutex);
    entries = rhs.entries;
    return *this;
  }

  Signal(Signal &&rhs) = default;
  Signal &operator=(Signal &&rhs) = default;

  Connection connect(const Slot &slot)
  {
    Lock lock(entriesMutex);
    auto conn = makeConnection();
    entries.emplace_back(Entry(slot, conn));
    return conn;
  }

  Connection connect(Slot &&slot)
  {
    Lock lock(entriesMutex);
    auto conn = makeConnection();
    entries.emplace_back(Entry(std::move(slot), conn));
    return conn;
  }

  template <typename Instance, typename MembFunc>
  Connection connect(Instance *instance, MembFunc Instance::*mf)
  {
    Lock lock(entriesMutex);
    auto slot = bindMf(instance, mf);
    auto conn = makeConnection();
    entries.emplace_back(Entry(slot, conn));
    return conn;
  }

  /// Connecting a signal will trigger all of its slots when this signal is triggered.
  Connection connect(Signal &signal)
  {
    Lock lock(entriesMutex);
    auto conn = makeConnection();
    entries.emplace_back(Entry(&signal, conn));
    return conn;
  }

  void clear()
  {
    Lock lock(entriesMutex);
    auto end = entries.end();
    for (auto it = entries.begin(); it != end; it++) {
      eraseEntry(it);
    }
  }

  void disconnect(Connection conn = nullptr)
  {
    if (!conn) {
      clear();
      return;
    }

    Lock lock(entriesMutex);

    auto end = entries.end();
    for (auto it = entries.begin(); it != end; it++) {
      if (it->conn() == conn) {
        eraseEntry(it);
      }
    }
  }

  void disconnect(Signal &signal)
  {
    Lock lock(entriesMutex);
    auto end = entries.end();
    for (auto it = entries.begin(); it != end; it++) {
      if (it->signal() == &signal) {
        eraseEntry(it);
      }
    }
  }

  void operator()(Args &&... args)
  {
    Lock lock(entriesMutex);
    for (auto &entry : entries) {
      auto *sig = entry.signal();
      if (sig) {
        (*sig)(std::forward<Args>(args)...);
      }
      else {
        entry.slot()(std::forward<Args>(args)...);
      }
    }
  }

  template <typename RetFunc = typename VoidableFunction<ReturnType>::func>
  void operator()(const RetFunc &retFunc, Args &&... args)
  {
    static_assert(!isVoidReturn(), "Must have non-void return type!");

    Lock lock(entriesMutex);
    for (auto &entry : entries) {
      auto *sig = entry.signal();
      if (sig) {
        (*sig)(retFunc, std::forward<Args>(args)...);
      }
      else {
        retFunc(entry.slot()(std::forward<Args>(args)...));
      }
    }
  }

private:
  Connection makeConnection()
  {
    auto conn = std::make_shared<ConnectionBase>();
    conn->deleter = [this, conn] { this->disconnect(conn); };
    return conn;
  }

  /// Expects entries container to be locked beforehand.
  void eraseEntry(typename Cont::iterator it)
  {
    auto conn = it->conn();
    if (conn) {
      conn->deleter = nullptr;
    }
    entries.erase(it);
  }

  template <typename Instance, typename MembFunc, std::size_t... Ns>
  Slot bindMf(Instance *instance, MembFunc Instance::*mf, Seq<Ns...>)
  {
    return std::bind(mf, instance, Placeholder<Ns>()...);
  }

  template <typename Instance, typename MembFunc>
  Slot bindMf(Instance *instance, MembFunc Instance::*mf)
  {
    return bindMf(instance, mf, MakeSeq<sizeof...(Args)>());
  }

  static constexpr bool isVoidReturn()
  {
    return std::is_void<ReturnType>::value;
  }

  Cont entries;
  std::mutex entriesMutex;
};

} // namespace sigs

#endif // SIGS_SIGNAL_SLOT_H
