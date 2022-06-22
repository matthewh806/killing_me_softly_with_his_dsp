from email.policy import default
from tkinter import Label
from flask_wtf import FlaskForm
from wtforms import TextAreaField, SubmitField, BooleanField, FloatField, IntegerField, HiddenField, FieldList
from flask_wtf.file import FileField, FileRequired

class EchoForm(FlaskForm):
    effect_name = HiddenField("echos")
    gain_in = FloatField('In Gain', default=0.8)
    gain_out = FloatField('Out Gain', default=0.9)
    n_echos = IntegerField("Num echos", default=1)
    delays = TextAreaField("Delays", default="60")
    decays = TextAreaField("Decays", default="0.4")
    submit = SubmitField("Add Effect")

class MoshForm(FlaskForm):
    effects = TextAreaField('effects')
    rendergif = BooleanField("Create GIF", default=False)
    submit = SubmitField('Mosh Image')

class UploadForm(FlaskForm):
    image = FileField(validators=[FileRequired()])
    submit = SubmitField('Upload Image')