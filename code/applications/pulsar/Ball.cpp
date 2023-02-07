/*
  ==============================================================================

    Ball.cpp
    Created: 25 Mar 2020 12:09:58am
    Author:  Matthew

  ==============================================================================
*/

#include "Ball.h"

using namespace OUS;

Physics::Ball::Ball(b2World& world, b2Vec2 pos, int noteNumber, float velocity, double radius, float density, float restitution)
{
    b2CircleShape circleShape;
    circleShape.m_p.Set(0, 0);
    circleShape.m_radius = static_cast<float>(radius);

    b2BodyDef circleBodyDef;
    circleBodyDef.type = b2_dynamicBody;
    circleBodyDef.position.Set(pos.x, pos.y);
    mBody = world.CreateBody(&circleBodyDef);

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &circleShape;
    fixtureDef.density = density;
    fixtureDef.restitution = restitution;
    fixtureDef.friction = 0.0f;
    mBody->CreateFixture(&fixtureDef);

    mBody->SetUserData(this);

    setMidiData(noteNumber, velocity);

    /*
     todo:
        this colour is currently unused because juce is relying on the
        debug draw for box2d which internally manages the colours for
        specific shapes.

        it would be nice to either randomise the ball colours or use some kind of
        scheme where its based on the note value (i.e. scaled with frequency)
     */
    //    mColour = juce::Colour(mRandom.nextInt (256), mRandom.nextInt (256), mRandom.nextInt (256));
}

Physics::Ball::~Ball()
{
    mBody->GetWorld()->DestroyBody(mBody);
}

void Physics::Ball::startContact()
{
    mContacting = true;
}

void Physics::Ball::endContact()
{
    mContacting = false;
}

bool Physics::Ball::isContacting()
{
    return mContacting;
}

b2Body* Physics::Ball::getBody()
{
    return mBody;
}

Physics::Ball::MidiData Physics::Ball::getMidiData() const
{
    return mMidiData;
}

void Physics::Ball::setMidiData(int noteNumber, float velocity)
{
    mMidiData.noteNumber = noteNumber;
    mMidiData.velocity = velocity;
}
