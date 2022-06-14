from flask_wtf import FlaskForm
from wtforms import TextAreaField, SubmitField
from flask_wtf.file import FileField, FileRequired

class MoshForm(FlaskForm):
    effects = TextAreaField('effects', default='[{"echos": {"gain_in": 0.8, "gain_out": 0.01, "delays": [60], "decays": [0.5]}}]')
    submit = SubmitField('Mosh Image')

class UploadForm(FlaskForm):
    image = FileField(validators=[FileRequired()])
    submit = SubmitField('Upload Image')