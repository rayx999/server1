#ifndef NET2_IO_SERVICE_POOL_HPP_
#define NET2_IO_SERVICE_POOL_HPP_
#include "base/base.hpp"
#include <boost/thread.hpp>
#include "thread/threadpool.hpp"
typedef shared_ptr<boost::asio::io_service> IOServicePtr;
/// A pool of io_service objects.
class IOServicePool : private boost::noncopyable {
public:
  /// Construct the io_service pool.
  explicit IOServicePool(size_t pool_size);

  /// Run all io_service objects in the pool.
  void Start();

  /// Stop all io_service objects in the pool.
  void Stop();

  /// Get an io_service to use.
  IOServicePtr get_io_service();

private:
  typedef shared_ptr<boost::asio::io_service::work> work_ptr;

  /// The pool of io_services.
  vector<IOServicePtr> io_services_;

  /// The work that keeps the io_services running.
  vector<work_ptr> work_;

  int next_io_service_;

  shared_ptr<ThreadPool> threadpool_;
  boost::mutex mutex_;
  int pool_size_;
};
#endif // NET2_IO_SERVICE_POOL_HPP_