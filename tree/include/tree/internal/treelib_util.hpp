#pragma once
#include <utility>


namespace internal 
{


    template<bool condition_t, typename TypeA, typename TypeB> 
    struct conditional_operator{};
    template<typename TypeA, typename TypeB> 
    struct conditional_operator<false, TypeA, TypeB>{ using type = TypeB; };
    template<typename TypeA, typename TypeB> 
    struct conditional_operator<true,  TypeA, TypeB>{ using type = TypeA; };
    

    template<typename T> struct value_ptr {
        using type = typename conditional_operator<sizeof(T) <= 8, T, T*>::type;
        static constexpr bool value = (sizeof(T) <= 8);
    };


    template<typename T>
    struct CompactifiedType {
        typename value_ptr<T>::type m_value;


        CompactifiedType() {
            memset( &m_value, 0x00, sizeof(decltype(m_value)) );
            return;
        }
        ~CompactifiedType() = default;



        T const& get() const {
            if constexpr ( value_ptr<T>::value == false ) {
                return *m_value;
            } else {
                return m_value;
            }
        }

        void set(T const& value) {
            if constexpr ( value_ptr<T>::value == false ) {
                if(m_value != nullptr) {
                    *m_value = value;
                } else {
                    m_value = new T{value};
                }
            } 
            else {
                m_value = value;
            }
            return;
        }


        void release() {
            if constexpr ( value_ptr<T>::value == false ) {
                delete m_value;
                m_value = nullptr;
                return;
            } 
            else {
                memset(&m_value, 0x00, sizeof(T));
                return;
            }
        }
    };


    template <typename T, typename = void> struct has_less 
        : 
        std::false_type {};
    template <typename T> struct has_less<  T, std::void_t< decltype(std::declval<const T&>() < std::declval<const T&>()) >  > 
        : 
        std::true_type {};
    template <typename T> inline constexpr bool 
        has_less_v = has_less<T>::value;


    template <typename T, typename = void> struct has_greater
        : 
        std::false_type {};
    template <typename T> struct has_greater<  T, std::void_t< decltype(std::declval<const T&>() > std::declval<const T&>()) >  > 
        : 
        std::true_type {};
    template <typename T> inline constexpr bool 
        has_greater_v = has_less<T>::value;


    template <typename T, typename = void> struct has_equal
        : 
        std::false_type {};
    template <typename T> struct has_equal<  T, std::void_t< decltype(std::declval<const T&>() < std::declval<const T&>()) >  > 
        : 
        std::true_type {};
    template <typename T> inline constexpr bool 
        has_equal_v = has_less<T>::value;
    
    
} /* namespace::internal */
