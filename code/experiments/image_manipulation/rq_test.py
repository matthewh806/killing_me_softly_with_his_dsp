from redis import Redis
import rq
import os
import time
from callbacks import report_success, report_failure

if __name__ == "__main__":
    CURRENT_DIRECTORY = os.path.dirname(os.path.realpath(__file__))
    INPUT_PATH = os.path.join(CURRENT_DIRECTORY, "input_images/perfect_blue_city.bmp")
    OUTPUT_PATH = os.path.join(CURRENT_DIRECTORY, "output_images/perfect_blue_city_async.bmp")

    queue = rq.Queue('soxmosh-tasks', connection=Redis())
    job = queue.enqueue('app.tasks.generate_image_task', INPUT_PATH, OUTPUT_PATH, on_success=report_success, on_failure=report_failure)
    print(job.get_id())
    print(job.meta)

    while not job.is_finished:
        print("Job", job.get_id(), "still running", "progress:", job.meta)
        time.sleep(1)

    print("Job", job.get_id(), "finished")
    print(job.meta)
