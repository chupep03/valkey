#pragma once

namespace Storage {

template <class... Ts>
struct Overloaded : Ts... {
    using Ts::operator()...;
    // алиас для каждой лямбды. без него получится что компилятор не решит от какого предка брать опреатор ()
};

template <class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>; // deduction guide: параметризация шаблона происходит тут,
                                        // так как лямбды анонимны и не можем их подставить

} // namespace Storage