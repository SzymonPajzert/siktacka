//
// Created by svp on 21.08.17.
//

#ifndef SIKTACKA_AWAITABLESOCKET_HPP
#define SIKTACKA_AWAITABLESOCKET_HPP

struct AwaitableSocket {
    virtual int get_sock() const = 0;

    virtual ~AwaitableSocket() = default;
};

#endif //SIKTACKA_AWAITABLESOCKET_HPP
