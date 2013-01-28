/***************************************************************************************************
 *  Copyright 2012 MaidSafe.net limited                                                            *
 *                                                                                                 *
 *  The following source code is property of MaidSafe.net limited and is not meant for external    *
 *  use.  The use of this code is governed by the licence file licence.txt found in the root of    *
 *  this directory and also on www.maidsafe.net.                                                   *
 *                                                                                                 *
 *  You are not free to copy, amend or otherwise use this source code without the explicit         *
 *  written permission of the board of directors of MaidSafe.net.                                  *
 **************************************************************************************************/

#ifndef MAIDSAFE_NFS_RESPONSE_MAPPER_H_
#define MAIDSAFE_NFS_RESPONSE_MAPPER_H_

#include <exception>
#include <future>
#include <string>
#include <utility>
#include <vector>

#include "maidsafe/common/utils.h"

namespace maidsafe {

namespace nfs {

template <typename FutureType, typename PromiseType>
class ResponseMapper {
 public:
  ResponseMapper();

  typedef std::pair<FutureType, PromiseType>  RequestPair;

  void push_back(RequestPair&& pending_pair);

 private:
  bool running();
  void Run();
  void Poll();

  mutable std::mutex mutex_;
  std::vector<RequestPair> active_requests_, pending_requests_;
  std::future<void> worker_future_;
};

template <typename FutureType, typename PromiseType>
ResponseMapper<FutureType, PromiseType>::ResponseMapper()
    : mutex_(),
      active_requests_(),
      pending_requests_(),
      worker_future_() {
  Run();
}

template <typename FutureType, typename PromiseType>
void ResponseMapper<FutureType, PromiseType>::push_back(RequestPair&& pending_pair) {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    pending_requests_.push_back(std::move(pending_pair));
  }
  Run();
}

template <typename FutureType, typename PromiseType>
void ResponseMapper<FutureType, PromiseType>::Run() {
  if (IsReady(worker_future_) || !running()) {  // FIXME extra check to avoid race condition
    worker_future_ = std::async(std::launch::async, Poll);
  }
}

template <typename FutureType, typename PromiseType>
bool ResponseMapper<FutureType, PromiseType>::running() {
  std::lock_guard<std::mutex> lock(mutex_);
  return (!active_requests_.empty());
}

template <typename FutureType, typename PromiseType>
void ResponseMapper<FutureType, PromiseType>::Poll() {
  while (running()) {
    for (auto& requests : active_requests_) {
      if (IsReady(requests.first)) {
        // Set promise
      }
    }
    {
      std::lock_guard<std::mutex> lock(mutex_);
      active_requests_.insert(active_requests_.end(), pending_requests_.begin(),  // FIXME need move
                              pending_requests_.end());
    }
  }
}

}  // namespace nfs

}  // namespace maidsafe

#endif  // MAIDSAFE_NFS_RESPONSE_MAPPER_H_