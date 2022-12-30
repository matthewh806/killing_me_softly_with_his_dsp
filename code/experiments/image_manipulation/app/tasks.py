from soxmosh import SoxMosh

def generate_gif_task(input_path, output_path, effects_json = None):
    print("Starting gif task")
    sox_mosh = SoxMosh(input_path)
    sox_mosh.databend_to_gif(output_path, effects_json)
    print("Finished gif task")

def generate_image_task(input_path, output_path, effects_json = None):
    print("Starting image task")
    sox_mosh = SoxMosh(input_path)
    sox_mosh.databend_image(output_path, effects_json)
    print("Finished image task")