/*
 * 1. エラー原因
 * 2. エラー詳細
 * 3. 想定外の場合はその種別の出力
 * 4. throw例外のサポート
 */

module;
#include <type_traits>
#ifdef MABOROUTU_ENABLE_EXCEPTION
#include <stdexcept>
#endif

export module maboroutu.error;

#ifdef MABOROUTU_ENABLE_EXCEPTION
#include <stdexcept>

namespace maboroutu {
// [[maboroutu_error]]
export class maboroutu_error : public std::runtime_error {
public: /*STRUCT_FIELD*/
protected:
private:
  using self_type = maboroutu_error;

  /*--:  *IMPLIMENT_FIELD*/
protected:
public:
  using std::runtime_error::runtime_error;
  maboroutu_error(const maboroutu_error &) = delete;
  maboroutu_error(maboroutu_error &&) = delete;

  auto operator=(maboroutu_error const &rhs) -> maboroutu_error & = default;
  auto operator=(maboroutu_error &&rhs) -> maboroutu_error & = default;
};
} // namespace maboroutu
#endif

namespace maboroutu {
// [[error_seed]]
export template <class ErrcT, class CharT, size_t SizeV> struct error_seed {
  ErrcT code;
  CharT message[SizeV];
  static constexpr size_t size = SizeV - 1;

  consteval error_seed(ErrcT arg_code, CharT const (&arg_message)[SizeV]) {
    code = arg_code;
    for (auto i = 0; i < size; i++) {
      message[i] = arg_message[i];
    }
    message[size] = '\n';
  }
};

// [[error]]
export template <class ErrcT, class CharT = char> class error {
public: /*STRUCT_FIELD*/
  static_assert(std::is_enum_v<ErrcT>, "ErrcT is enum.");

  using code_type = ErrcT;
  using message_type = CharT const *;
  template <size_t SizeV> using seed_type = error_seed<ErrcT, CharT, SizeV>;

protected:
private:
  using self_type = error;

  struct container_t {
    code_type code;
    message_type message;
  };

  container_t const *_value;

  /*--:  *IMPLIMENT_FIELD*/
protected:
  consteval explicit error(container_t const *value) : _value(value) {
#ifdef MABOROUTU_ENABLE_EXCEPTION
    throw maboroutu_error{value->message};
#endif
  }

public:
  error() = delete;
  error(error const &) = default;
  error(error &&) = default;
  ~error() = default;

  constexpr auto code(this self_type &self) noexcept -> code_type {
    return self._value->message;
  }

  constexpr auto what(this self_type &self) noexcept -> message_type {
    return self._value->message;
  }

  template <error_seed SeedV> static consteval auto make() -> self_type {
    static constexpr container_t value{.code = SeedV.code,
                                       .message = SeedV.message};
    return self_type(&value);
  }

  auto operator=(error const &rhs) -> error & = default;
  auto operator=(error &&rhs) -> error & = default;
};

} // namespace maboroutu
