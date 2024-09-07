# Generic Job Queues
generic job queue for single producer-consumer and multiple producer-consumer job queueing and processing.
For mpmc the design is to have one queue per producer-consumer pair with an access token, any other atomic
methods to access single queue concurrently invovles some sort of spinning to achieve concurrency amongst
producers during queueing and amongst consumers during dequeueing which is similar to using spinlocks.
