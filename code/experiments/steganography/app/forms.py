from flask_wtf import FlaskForm
from wtforms import FileField, SubmitField
from flask_wtf.file import FileRequired

class TransmitterForm(FlaskForm):
    # TODO:
    #   - low pass freq
    #   - bandpass low / high cutoff
    #   - filter order
    #   - carrier freq
    #   - mod index
    # output path?
    base_audio = FileField(validators=[FileRequired()])
    secret_audio = FileField(validators=[FileRequired()])
    submit = SubmitField("Hide audio")