from flask_wtf import FlaskForm
from wtforms import TextAreaField, SubmitField

class MoshForm(FlaskForm):
    effects = TextAreaField('effects', default='[{"echos": {"gain_in": 0.8, "gain_out": 0.01, "delays": [60], "decays": [0.5]}}]')
    submit = SubmitField('Mosh Image')