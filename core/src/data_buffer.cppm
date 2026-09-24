module;
#include <bit>
#include <cassert>
#include <compare>
#include <concepts>
#include <iterator>
#include <limits>
#include <memory>
#include <span>
#include <stdexcept>
#include <type_traits>
export module maboroutu.data_buffer;
export import maboroutu.core;
export import maboroutu.error;

import maboroutu.data_source;

namespace maboroutu {

// [[segmented_span_iterator]]
export template <class DependT, class T, bool IsConst>
class segmented_span_iterator {
 public:
   using iterator_concept = std::random_access_iterator_tag;
   using difference_type = std::ptrdiff_t;
   using value_type = T;
   using pointer = typename std::conditional_t<IsConst, T const, T> *;
   using reference = typename std::conditional_t<IsConst, T const, T> &;
   using iterator_category = std::random_access_iterator_tag;

 protected:
 private:
   using self_type = segmented_span_iterator;
   using dependency_type = DependT;
   using size_type = std::remove_cvref_t<typename DependT::size_type>;
   friend dependency_type;

   using data_pointer =
       std::conditional_t<IsConst, dependency_type const, dependency_type> *;

   data_pointer _data = nullptr;
   size_type _index = 0;

   /*--:  *IMPLIMENT_FIELD*/
   segmented_span_iterator(data_pointer data, size_type index)
       : _data(data), _index(index) {}

 protected:
 public:
   segmented_span_iterator() = default;
   segmented_span_iterator(segmented_span_iterator const &other)
       : _data(other._data), _index(other._index) {}
   segmented_span_iterator(segmented_span_iterator &&other) noexcept
       : _data(other._data), _index(other._index) {}
   segmented_span_iterator(
       segmented_span_iterator<DependT, T, false> const &other)
      requires IsConst
       : _data(other._data), _index(other._index) {}
   ~segmented_span_iterator() = default;

   auto operator++(this self_type &self) noexcept -> segmented_span_iterator & {
      ++self._index;
      return self;
   }
   auto operator++(this self_type &self, int) noexcept
       -> segmented_span_iterator {
      self_type ret_value = self;
      ++self;
      return ret_value;
   }

   auto operator--(this self_type &self) noexcept -> segmented_span_iterator & {
      --self._index;
      return self;
   }
   auto operator--(this self_type &self, int) noexcept
       -> segmented_span_iterator {
      self_type ret_value = self;
      --self;
      return ret_value;
   }

   [[nodiscard]] friend auto operator+(self_type const &lhs,
                                       difference_type rhs) noexcept
       -> self_type {
      return self_type(lhs._data, lhs._index + rhs);
   }
   [[nodiscard]] friend auto operator+(difference_type lhs,
                                       self_type const &rhs) noexcept
       -> self_type {
      return self_type(rhs._data, rhs._index + lhs);
   }
   [[nodiscard]] friend auto operator-(self_type const &lhs,
                                       difference_type rhs) noexcept
       -> self_type {
      return self_type(lhs._data, lhs._index - rhs);
   }
   [[nodiscard]] friend auto operator-(self_type const &lhs,
                                       self_type const &rhs) noexcept
       -> difference_type {
      assert(lhs._data == rhs._data);
      return (lhs._index >= rhs._index)
                 ? difference_type(lhs._index - rhs._index)
                 : -difference_type(rhs._index - lhs._index);
   }

   auto operator+=(this self_type &self, difference_type index) noexcept
       -> self_type & {
      self._index += index;
      return self;
   }
   auto operator-=(this self_type &self, difference_type index) noexcept
       -> self_type & {
      self._index -= index;
      return self;
   }

   auto operator*() const noexcept -> reference {
      assert(_data != nullptr &&
             "segmented_span_iterator: this iterator is nullptr.");
      assert(_index < _data->size() &&
             "segmented_span_iterator: out of range error in "
             "operator*");
      return (*_data)[_index];
   }

   auto operator->() const noexcept -> pointer {
      assert(_data != nullptr &&
             "segmented_span_iterator: this iterator is nullptr.");
      assert(_index < _data->size() &&
             "segmented_span_iterator: out of range error in "
             "operator->");
      return &(*_data)[_index];
   }

   auto operator[](difference_type index) const noexcept -> reference {
      assert(_data != nullptr &&
             "segmented_span_iterator: this iterator is nullptr.");
      assert((_index + index) < _data->size() &&
             "segmented_span_iterator: out of range error in "
             "operator->");
      return (*_data)[_index + index];
   }

   template <bool LocIsConst>
   friend auto operator==(
       segmented_span_iterator const &lhs,
       segmented_span_iterator<DependT, T, LocIsConst> const &rhs) noexcept
       -> bool {
      return (lhs._data == rhs._data) && (lhs._index == rhs._index);
   }
   template <bool LocIsConst>
   friend auto
   operator<=>(segmented_span_iterator const &lhs,
               segmented_span_iterator<DependT, T, LocIsConst> const &rhs)
       -> std::strong_ordering {
      assert(lhs._data == rhs._data &&
             "iterator does not match referenced container.");
      return lhs._index <=> rhs._index;
   }
   auto operator=(segmented_span_iterator const &other)
       -> segmented_span_iterator & = default;
   auto operator=(segmented_span_iterator &&other)
       -> segmented_span_iterator & = default;
   auto operator=(segmented_span_iterator<DependT, T, false> const &other)
       -> segmented_span_iterator &
      requires IsConst
   {
      _data = other._data;
      _index = other._index;
      return *this;
   }
};

export template <class T, std::size_t UnitSize>
   requires(std::has_single_bit(UnitSize))
// [[segmented_span]]
class segmented_span {
 public: /*STRUCT_FIELD*/
   using value_type = T;
   using size_type = std::size_t;

 protected:
 private:
   using self_type = segmented_span;
   template <class, class, bool> friend class segmented_span_iterator;

   std::span<std::span<value_type, UnitSize>> _container{};
   size_type _size = 0;

   /*--:  *IMPLIMENT_FIELD*/
 protected:
 public:
   using iterator = segmented_span_iterator<self_type, T, false>;
   using const_iterator = segmented_span_iterator<self_type, T, true>;

   segmented_span() = default;
   segmented_span(segmented_span const &) = default;
   segmented_span(segmented_span &&) = default;
   segmented_span(std::span<std::span<value_type, UnitSize>> span,
                  size_type size)
       : _container(span), _size(size) {
      // NOTE: 実行時にもthrowによる検証を行うが、
      //       本来はassertのみの制約でも良い。
      bool status =
          span.size() <=
              (std::numeric_limits<decltype(span.size())>::max() / UnitSize) &&
          _size <= (span.size() * UnitSize);
      assert(status && "out of range size than actual span size.");
      if (!status) {
         throw std::invalid_argument(
             "out of range size than actual span size.");
      }
   }
   segmented_span(std::span<std::span<value_type, UnitSize>> span)
       : _container(span), _size(UnitSize * span.size()) {}
   ~segmented_span() = default;

   [[nodiscard]] auto size() const -> size_type { return _size; }
   [[nodiscard]] auto empty() const -> bool { return _size == 0; }

   [[nodiscard]] auto segments() const
       -> std::span<std::span<value_type, UnitSize> const> {
      return _container;
   }

   template <class Self>
   [[nodiscard]] auto at(this Self &self, size_type index)
       -> std::conditional_t<std::is_const_v<Self>, value_type const,
                             value_type> & {
      if (index >= self._size) [[unlikely]] {
         throw std::out_of_range("segmented_span::at: index out of range");
      }
      return self._container[index / UnitSize][index % UnitSize];
   }
   template <class Self>
   [[nodiscard]] auto operator[](this Self &self, size_type index)
       -> std::conditional_t<std::is_const_v<Self>, value_type const,
                             value_type> & {
      assert((index < self._size) &&
             "segmented_span::operator[]: index out of range");
      return self._container[index / UnitSize][index % UnitSize];
   }

   template <class Self>
   constexpr auto begin(this Self &self) noexcept
       -> std::conditional_t<std::is_const_v<Self>, const_iterator, iterator> {
      if constexpr (std::is_const_v<Self>) {
         return const_iterator(&self, 0);
      } else {
         return iterator(&self, 0);
      }
   }
   constexpr auto cbegin(this self_type const &self) noexcept
       -> const_iterator {
      return const_iterator(&self, 0);
   }
   template <class Self>
   constexpr auto end(this Self &self) noexcept
       -> std::conditional_t<std::is_const_v<Self>, const_iterator, iterator> {
      if constexpr (std::is_const_v<Self>) {
         return const_iterator(&self, self._size);
      } else {
         return iterator(&self, self._size);
      }
   }
   constexpr auto cend(this self_type const &self) noexcept -> const_iterator {
      return const_iterator(&self, self._size);
   }

   auto operator=(segmented_span const &rhs) -> segmented_span & = default;
   auto operator=(segmented_span &&rhs) -> segmented_span & = default;
};

using segmented_span_check = segmented_span<int, 1024>;
static_assert(std::random_access_iterator<segmented_span_check::iterator>, "");
static_assert(std::random_access_iterator<segmented_span_check::const_iterator>,
              "");

namespace errc {
enum class data_buffer {
   out_of_range,
   operation_failure,
};
}

export template <class T>
using data_buffer_result = result<T, errc::data_buffer>;

export template <class T>
concept data_buffer =
    writable_data_source<T> &&
    requires(T &buf, std::size_t n, region r, std::span<std::byte const> data) {
       { buf.append(data) } -> std::same_as<data_buffer_result<region>>;
       { buf.grow(n) } -> std::same_as<data_buffer_result<region>>;
       {
          buf.view(r)
       } -> std::same_as<data_buffer_result<std::span<std::byte>>>;
    };

// NOTE: concept検証兼 posix /dev/null の模倣
export struct null_data_buffer {
   template <class T> using result_type = data_source_result<T>;

   [[nodiscard]] static auto size() -> result_type<std::size_t> { return 0; }
   [[nodiscard]] static auto read(region r) -> result_type<byte_array> {
      return byte_array{
          .value = std::make_unique<byte_array::value_type>(r.size),
          .size = r.size,
      };
   }
   [[nodiscard]] auto write(region, std::span<std::byte const>)
       -> result_type<void> {
      return {};
   }

   static auto append(std::span<std::byte const> n)
       -> data_buffer_result<region> {
      return region{
          .offset = 0,
          .size = 0,
      };
   }
   static auto grow(std::size_t n) -> data_buffer_result<region> {
      return region{
          .offset = 0,
          .size = 0,
      };
   }
   static auto view(region r) -> data_buffer_result<std::span<std::byte>> {
      return make_unexpected(
          data_buffer_result<void>::error_type::code_type::out_of_range);
   }
};
static_assert(data_buffer<null_data_buffer>, "");

} // namespace maboroutu
