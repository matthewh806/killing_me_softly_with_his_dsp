from flask_wtf import FlaskForm
from wtforms import TextAreaField, SubmitField, BooleanField, FloatField, IntegerField, HiddenField, FieldList
from wtforms.validators import NumberRange
from flask_wtf.file import FileField, FileRequired

class EffectForm(FlaskForm):
    effect_name = HiddenField()
    submit = SubmitField("Add Effect")

class CompandForm(EffectForm):
    effect_name = HiddenField("compand")
    attack_time = FloatField("Attack Time", default=0.3)
    decay_time = FloatField("Decay Time", default=0.8)

class EchoForm(EffectForm):
    gain_in = FloatField('In Gain', default=0.8)
    gain_out = FloatField('Out Gain', default=0.9)
    n_echos = IntegerField("Num echos", default=1)
    delays = TextAreaField("Delays", default="60")
    decays = TextAreaField("Decays", default="0.4")

class OverdriveForm(EffectForm):
    gain_db = FloatField("Gain (dB)", default=20.0)
    colour = FloatField("Colour (dB)", default=20.0)

class PhaserForm(EffectForm):
    # TODO: Add modulation shape param (dropdown?)
    gain_in = FloatField("In Gain", [NumberRange(min=0.0, max=1.0)], default=0.8)
    gain_out = FloatField("Out Gain", [NumberRange(min=0.0, max=1.0)], default=0.74)
    delay = FloatField("Delay (ms)", [NumberRange(min=0.0, max=5.0)], default=3)
    decay = FloatField("Decay", [NumberRange(min=0.0, max=0.5)], default=0.4)
    speed = FloatField("Speed (Hz)", [NumberRange(min=0.1, max=2.0)], default=0.5)

class ReverbForm(EffectForm):
    reverberance = FloatField("Reverberance", default=50)
    high_freq_damping = IntegerField("High Frequency Damping (%)", default=50)
    room_scale = IntegerField("Room Scale (%)", default=100)
    pre_delay = FloatField("Pre Delay (ms)", default=0.0)
    wet_gain = FloatField("Wet Gain (dB)", default=0.0)

class ClearForm(FlaskForm):
    submit = SubmitField("Clear Effects")

class MoshForm(FlaskForm):
    effects = TextAreaField('Effects List')
    rendergif = BooleanField("Create GIF", default=False)
    submit = SubmitField('Mosh Image')

class UploadForm(FlaskForm):
    image = FileField(validators=[FileRequired()])
    submit = SubmitField('Upload Image')