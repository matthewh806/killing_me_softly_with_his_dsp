
def report_success(job, connection, result, *args, **kwargs):
    print("Success callback for job:", job.get_id(), "with result:", result)

def report_failure(job, connection, type, value, traceback):
    print("Failure callback for job:", job.get_id())