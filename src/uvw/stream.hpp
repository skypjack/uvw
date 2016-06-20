#pragma once


namespace uvw {


template<class T>
class Connection: public Resource<T> {
    using Resource<T>::Resource;
};


template<class T>
class Stream: public Connection<T> {
    using Connection<T>::Connection;
};


}
