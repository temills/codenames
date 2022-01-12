from . import db

#need secret code at end of experiment?
#do they have a prolific id?


class Subject(db.Model):
    __tablename__ = 'subjects'
    
    subject_id = db.Column(db.String, primary_key=True)
    completion_code = db.Column(db.String)
    age = db.Column(db.String)
    gender = db.Column(db.String)
    nationality = db.Column(db.String)
    country = db.Column(db.String)
    student = db.Column(db.String)
    language = db.Column(db.String)
    education = db.Column(db.String)
    check_num_dif = db.Column(db.String)
    check_words = db.Column(db.String)

    def __repr__(self):
        return '<Subject %r>' % self.id


class Trial(db.Model):
    __tablename__ = 'trials'
    row_id = db.Column(db.String, primary_key=True)
    subject_id = db.Column(db.String)
    rt = db.Column(db.String)
    clue = db.Column(db.String)
    trial_order = db.Column(db.String)
    chosen_words = db.Column(db.String)
    words = db.Column(db.String)
    
    def __repr__(self):
        return '<Subject %r>' % self.id