#pragma once

namespace Engine {
template <typename M> struct GetPointerType {
    template <typename C, typename T> static T getType(T C::*v);

    typedef decltype(getType(static_cast<M>(nullptr))) type;
};

template <typename Var, Var var> struct GetVarTraits;

template <typename C, typename T, T C::*Var> struct GetVarTraits<T C::*, Var> {
    using klass = C;
};
} // namespace Engine
