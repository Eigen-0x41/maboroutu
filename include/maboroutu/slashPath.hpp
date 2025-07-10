#pragma once

#include <cassert>
#include <compare>
#include <concepts>
#include <iterator>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
namespace maboroutu {
template <class T>
concept SlashPathTraitsConcepts = requires(T &Ins) {
  typename T::string_type;
  typename T::string_view_type;
};

namespace {
template <class T, SlashPathTraitsConcepts TraitsT> class SlashPathIterator {
public:
  using this_type = SlashPathIterator<T, TraitsT>;

  using value_type = typename TraitsT::string_view_type;
  using difference_type = typename value_type::size_type;
  using iterator_concept = typename std::bidirectional_iterator_tag;

  static_assert(std::signed_integral<difference_type>,
                "difference_type is unsigned.");

private:
  friend T;

  value_type const String;
  value_type Current;
  difference_type Begin;
  difference_type End;

  SlashPathIterator(value_type String, difference_type const Begin)
      : String(String), Begin(Begin) {
    if (Begin == value_type::npos) {
      makeSentinel();
    } else {
      End = this->String.find(T::PreferredSeparator, Begin);
      if (End == value_type::npos) {
        makeSentinel();
      }
    }
    updateCurrent();
  }

  difference_type step(difference_type Pos = value_type::npos) const {
    return String.find(T::PreferredSeparator, Pos);
  }
  difference_type rstep(difference_type Pos = value_type::npos) const {
    return String.rfind(T::PreferredSeparator, Pos) + 1;
  }

  void makeSentinel() noexcept {
    Begin = value_type::npos;
    End = value_type::npos;
  }

  void makeDirectoryBack() noexcept {
    Begin = String.size();
    End = String.size();
  }

  bool isDirectoryBackPos(difference_type Pos) const noexcept {
    return Pos >= String.size();
  }

  bool isStrSizeZero() const noexcept { return Begin == End; }
  bool isFront() const noexcept { return Begin == 0; }
  bool isBack() const noexcept { return End == value_type::npos; }
  bool isDirectoryBack() const noexcept {
    return isDirectoryBackPos(Begin) && isDirectoryBackPos(End);
  }
  bool isSentinel() const noexcept {
    return (Begin == value_type::npos) && (End == value_type::npos);
  }

  difference_type size() const noexcept { return End - Begin; }

  void updateCurrent() noexcept {
    if (isStrSizeZero()) [[unlikely]] {
      Current = "";
      return;
    }
    if (isBack()) [[unlikely]] {
      Current = this->String.substr(Begin);
      return;
    }
    Current = this->String.substr(Begin, size());
  }

protected:
public:
  SlashPathIterator() = default;
  SlashPathIterator(this_type const &Val)
      : String(Val.String), Current(Val.Current), Begin(Val.Begin),
        End(Val.End) {}
  SlashPathIterator(this_type &&Val)
      : String(std::move(Val.String)), Current(std::move(Val.Current)),
        Begin(std::move(Val.Begin)), End(std::move(Val.End)) {}

  this_type &operator=(this_type const &Val) {
    String = Val, String;
    Current = Val.Current;
    Begin = Val.Begin;
    End = Val.End;
  }
  this_type &operator=(this_type &&Val) {
    String = std::move(Val, String);
    Current = std::move(Val.Current);
    Begin = std::move(Val.Begin);
    End = std::move(Val.End);
  }

  this_type &operator++() noexcept {
    do {
      if (isSentinel()) {
        Begin = 0;
        End = step();
        updateCurrent();
        return *this;
      }

      if (isBack() || isDirectoryBack()) {
        makeSentinel();
        updateCurrent();
        return *this;
      }

      Begin = End + 1;

      if (isDirectoryBackPos(Begin)) {
        makeDirectoryBack();
        updateCurrent();
        return *this;
      }

      End = step(Begin);
    } while (isStrSizeZero());
    updateCurrent();
    return *this;
  }
  this_type operator++(int) noexcept {
    this_type RetValue = *this;
    operator++();
    return RetValue;
  }

  this_type &operator--() noexcept {
    do {
      if (isSentinel()) {
        Begin = rstep();
        if (isDirectoryBackPos(Begin)) {
          makeDirectoryBack();
        }
        updateCurrent();
        return *this;
      }

      if (isFront()) {
        makeSentinel();
        updateCurrent();
        return *this;
      }

      End = Begin - 1;
      if (End == 0) {
        Begin = 0;
        break;
      }
      Begin = rstep(End - 1);
      if (Begin == value_type::npos) {
        Begin = 0;
      }
    } while (isStrSizeZero());
    updateCurrent();
    return *this;
  }
  this_type operator--(int) noexcept {
    this_type RetValue = *this;
    operator--();
    return RetValue;
  }

  value_type const &operator*() const noexcept { return Current; }
  value_type const *operator->() const noexcept { return &Current; }

  friend bool operator==(this_type const &Left,
                         this_type const &Right) noexcept {
    return isMatchIterator(Left, Right) && (Left.Begin == Right.Begin) &&
           (Left.End == Right.End);
  }

  friend bool isMatchIterator(this_type const &Left,
                              this_type const &Right) noexcept {
    return Left.String == Right.String;
  }
};
} // namespace

template <SlashPathTraitsConcepts TraitsT> class BasicSlashPath {
public:
  using this_type = BasicSlashPath<TraitsT>;
  using trait_type = TraitsT;

  using string_type = typename trait_type::string_type;
  using string_view_type = typename trait_type::string_view_type;
  using iterator = SlashPathIterator<this_type, trait_type>;

  static constexpr typename string_type::value_type PreferredSeparator = '/';

private:
protected:
  string_type Path;

public:
  BasicSlashPath() = default;
  BasicSlashPath(this_type const &Path) = default;
  BasicSlashPath(this_type &&Path) = default;
  template <SlashPathTraitsConcepts LocTraitsT>
  BasicSlashPath(BasicSlashPath<LocTraitsT> const &Path)
      : Path(Path.getString()) {}
  template <class... ArgsT>
  BasicSlashPath(ArgsT &&...Args) : Path(std::forward<ArgsT>(Args)...) {}
  ~BasicSlashPath() = default;
  this_type &operator=(this_type const &) = default;

  template <SlashPathTraitsConcepts LocTraitsT>
  bool operator==(BasicSlashPath<LocTraitsT> const &Right) {
    return compare(Right);
  }
  template <SlashPathTraitsConcepts LocTraitsT>
  std::strong_ordering operator<=>(BasicSlashPath<LocTraitsT> const &Right) {
    return compare(Right) <=> 0;
  }

  iterator begin() { return iterator(Path, 0); }
  iterator beginOrRootChild() {
    return (hasRootDirectory()) ? ++begin() : begin();
  }
  iterator end() { return iterator(Path, string_type::npos); }
  iterator endOrDirectoryLast() { return (isDirectory()) ? --end() : end(); }

  bool isDirectory() const { return Path.back() == PreferredSeparator; }
  bool hasRootDirectory() const { return trait_type::isHasRootDirectory(Path); }
  bool isEmpty() { return begin() == end(); }
  bool isOnlyName() {
    if (isEmpty()) {
      return false;
    }
    return begin() == endOrDirectoryLast();
  }

  int compare(string_type const &S) const { return compare(this_type(S)); }
  int compare(string_view_type Sv) const { return compare(this_type(Sv)); }
  template <SlashPathTraitsConcepts LocTraitsT>
  int compare(BasicSlashPath<LocTraitsT> const &Path) const {
    int Count = 0;
    for (auto const &&[IT, IL] : std::views::zip(*this, Path)) {
      auto Condition = IT <=> IL;
      if (Condition > 0) {
        return Count;
      }
      if (Condition < 0) {
        return -Count;
      }
      Count++;
    }
    return 0;
  }
  string_type const &getString() const noexcept { return Path; }

  string_type const &detach() noexcept {
    static_assert(!trait_type::IsStringTypeView, "This type is View!!");

    Path.resize(Path.rfind(PreferredSeparator));
    return Path;
  }
  string_type const &detach(size_t const Size) noexcept {
    static_assert(!trait_type::IsStringTypeView, "This type is View!!");

    for (auto const I : std::views::repeat(0, Size)) {
      Path.resize(Path.rfind(PreferredSeparator));
    }
    return Path;
  }
  string_type const &append(string_view_type Sv) noexcept {
    static_assert(!trait_type::IsStringTypeView, "This type is View!!");

    Path += PreferredSeparator;
    Path += Sv;
    return Path;
  }
  string_type const &operator/=(string_view_type Sv) noexcept {
    return append(Sv);
  }
  string_type const &assign(string_view_type Sv) noexcept {
    static_assert(!trait_type::IsStringTypeView, "This type is View!!");

    Path += Sv;
    return Path;
  }
  string_type const &operator+=(string_view_type Sv) noexcept {
    return assign(Sv);
  }
};

template <SlashPathTraitsConcepts TraitsT> class BasicSlashPathView {
public:
  using this_type = BasicSlashPath<TraitsT>;
  using trait_type = TraitsT;

  using string_view_type = typename trait_type::string_view_type;
  using iterator = SlashPathIterator<this_type, trait_type>;

  static constexpr typename string_view_type::value_type PreferredSeparator =
      '/';

private:
protected:
  string_view_type Path;

public:
  BasicSlashPathView() = default;
  BasicSlashPathView(this_type const &Path) = default;
  BasicSlashPathView(this_type &&Path) = default;
  template <SlashPathTraitsConcepts LocTraitsT>
  BasicSlashPathView(BasicSlashPath<LocTraitsT> const &Path)
      : Path(Path.getString()) {}
  template <class... ArgsT>
  BasicSlashPathView(ArgsT &&...Args) : Path(std::forward<ArgsT>(Args)...) {}
  ~BasicSlashPathView() = default;
  this_type &operator=(this_type const &) = default;

  template <SlashPathTraitsConcepts LocTraitsT>
  bool operator==(BasicSlashPath<LocTraitsT> const &Right) {
    return compare(Right);
  }
  template <SlashPathTraitsConcepts LocTraitsT>
  std::strong_ordering operator<=>(BasicSlashPath<LocTraitsT> const &Right) {
    return compare(Right) <=> 0;
  }

  iterator begin() { return iterator(Path, 0); }
  iterator beginOrRootChild() {
    return (hasRootDirectory()) ? ++begin() : begin();
  }
  iterator end() { return iterator(Path, string_view_type::npos); }
  iterator endOrDirectoryLast() { return (isDirectory()) ? --end() : end(); }

  bool isDirectory() const { return Path.back() == PreferredSeparator; }
  bool hasRootDirectory() const { return trait_type::isHasRootDirectory(Path); }
  bool isEmpty() { return begin() == end(); }
  bool isOnlyName() {
    if (isEmpty()) {
      return false;
    }
    return begin() == endOrDirectoryLast();
  }

  int compare(string_view_type Sv) const { return compare(this_type(Sv)); }
  template <SlashPathTraitsConcepts LocTraitsT>
  int compare(BasicSlashPath<LocTraitsT> const &Path) const {
    int Count = 0;
    for (auto const &&[IT, IL] : std::views::zip(*this, Path)) {
      auto Condition = IT <=> IL;
      if (Condition > 0) {
        return Count;
      }
      if (Condition < 0) {
        return -Count;
      }
      Count++;
    }
    return 0;
  }
  string_view_type const getString() const noexcept { return Path; }
};

template <class CharT, class TraitsT = typename std::char_traits<CharT>,
          class Allocator = typename std::allocator<CharT>>
struct SlashPathTraits {
  using string_type = typename std::basic_string<CharT, TraitsT, Allocator>;
  using string_view_type = typename std::basic_string_view<CharT, TraitsT>;

  static constexpr bool IsStringTypeView = false;

  static bool
  isHasRootDirectory(string_view_type const Path,
                     string_type::value_type PreferredSeparator) noexcept {
    // Unix path root check.
    if (Path.size() == 0) {
      return false;
    }
    return Path[0] == PreferredSeparator;
  }
};
using SlashPath = BasicSlashPath<SlashPathTraits<char>>;
using wSlashPath = BasicSlashPath<SlashPathTraits<wchar_t>>;
using u8SlashPath = BasicSlashPath<SlashPathTraits<char8_t>>;
using u16SlashPath = BasicSlashPath<SlashPathTraits<char16_t>>;
using u32SlashPath = BasicSlashPath<SlashPathTraits<char32_t>>;

using SlashPathView = BasicSlashPathView<SlashPathTraits<char>>;
using wSlashPathView = BasicSlashPathView<SlashPathTraits<wchar_t>>;
using u8SlashPathView = BasicSlashPathView<SlashPathTraits<char8_t>>;
using u16SlashPathView = BasicSlashPathView<SlashPathTraits<char16_t>>;
using u32SlashPathView = BasicSlashPathView<SlashPathTraits<char32_t>>;
} // namespace maboroutu

namespace std {
template <maboroutu::SlashPathTraitsConcepts T>
struct hash<maboroutu::BasicSlashPath<T>> {
  using is_transparent = void;

  size_t operator()(maboroutu::BasicSlashPath<T> PathView) const {
    return std::hash<typename maboroutu::BasicSlashPath<T>::string_view_type>{}(
        PathView.getString());
  }
  size_t operator()(
      typename maboroutu::BasicSlashPath<T>::string_view_type StrPath) const {
    return std::hash<typename maboroutu::BasicSlashPath<T>::string_view_type>{}(
        StrPath);
  }
};

template <maboroutu::SlashPathTraitsConcepts T>
struct hash<maboroutu::BasicSlashPathView<T>> {
  using is_transparent = void;

  size_t operator()(maboroutu::BasicSlashPath<T> PathView) const {
    return std::hash<typename maboroutu::BasicSlashPath<T>::string_view_type>{}(
        PathView.getString());
  }
  size_t operator()(
      typename maboroutu::BasicSlashPath<T>::string_view_type StrPath) const {
    return std::hash<typename maboroutu::BasicSlashPath<T>::string_view_type>{}(
        StrPath);
  }
};
} // namespace std
