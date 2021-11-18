#include <iostream>
#include "SFML/Graphics.hpp"
#include "GUI/gui_displayobjects.h"
#include <math.h>

//DisplayObject
GUI::DisplayObject::DisplayObject() :
    state(StartPhase),
    waitStartTime(0.f),
    waitEndTime(0.f),
    transmissionFactor(0.1f),
    position(sf::Vector2f(0, 0)),
    scale(sf::Vector2f(0, 0)),
    transparency(255),
    drawStatus(true),
    startTime(0),
    endTime(0)
{
}

GUI::DisplayObject::~DisplayObject()
{
}

void GUI::DisplayObject::SetPhase(GUI::PhaseStage stage_, bool quickTransmit)
{
    state = stage_;

    if(quickTransmit)
    {
        DisplayPhase phase;
        if(state == StartPhase)
            phase = startPhase;
        else if(state == DefaultPhase)
            phase = defaultPhase;
        else if(state == InactivePhase)
            phase = inactivePhase;

        position = phase.position;
        scale = phase.scale;
        transparency = phase.transparency;
    }
}

void GUI::DisplayObject::Reset()
{
    state = StartPhase;
    waitStartTime = startTime;
    waitEndTime = endTime;

    position = startPhase.position;
    scale = startPhase.scale;
    transparency = startPhase.transparency;
}

void GUI::DisplayObject::EditPhase(DisplayPhase phase_, PhaseStage stage_)
{
    if(stage_ == StartPhase)
        startPhase = phase_;
    else if(stage_ == DefaultPhase)
        defaultPhase = phase_;
    else if(stage_ == InactivePhase)
        inactivePhase = phase_;
}

void GUI::DisplayObject::SetInactive()
{
    state = InactivePhase;
    waitStartTime = 0;
}

void GUI::DisplayObject::SetSinglePhasePosition(sf::Vector2f position_)
{
    startPhase.position = defaultPhase.position = inactivePhase.position = position_;
}

void GUI::DisplayObject::SetDrawStatus(bool status_)
{
    drawStatus = status_;
}

void GUI::DisplayObject::Update(float deltaTime_)
{
    DisplayPhase phase;

    if(state == StartPhase)
        phase = startPhase;
    else if(state == DefaultPhase)
        phase = defaultPhase;
    else if(state == InactivePhase)
        phase = inactivePhase;
    else state = StartPhase;

    if(waitEndTime > 0 && state == InactivePhase)
    {
        waitEndTime -= deltaTime_;
        if(waitEndTime < 0)
            waitEndTime = 0;

        phase = defaultPhase;
    }

    if(transmissionFactor <= 0 || state == StartPhase)
    {
        position = phase.position;
        scale = phase.scale;
        transparency = phase.transparency;
    }
    else if(deltaTime_ > 0)
    {
        //Position
        if(position != phase.position)
        {
            sf::Vector2f dist = phase.position - position;
            float distLength = std::sqrt(dist.x * dist.x + dist.y * dist.y);

            if(distLength > 0)
            {
                sf::Vector2f unitVec = dist / distLength;
                float transmit = distLength * deltaTime_ / transmissionFactor;
                if(transmit < MIN_TRANSMISSION_VALUE)
                    transmit = MIN_TRANSMISSION_VALUE;

                position += unitVec * transmit;

                dist = phase.position - position;
                if(dist.x * unitVec.x + dist.y * unitVec.y < 0)
                    position = phase.position;
            }
        }

        //Scale
        if(scale != phase.scale)
        {
            sf::Vector2f difference = phase.scale - scale;
            float differenceLength = std::sqrt(difference.x * difference.x + difference.y * difference.y);

            if(differenceLength > 0)
            {
                sf::Vector2f unitVec = difference / differenceLength;
                float transmit = differenceLength * deltaTime_ / transmissionFactor;
                if(transmit < MIN_TRANSMISSION_SCALE)
                    transmit  = MIN_TRANSMISSION_SCALE;

                scale += unitVec * transmit;

                difference = phase.scale - scale;
                if(unitVec.x * difference.x + unitVec.y * difference.y < 0)
                    scale = phase.scale;
            }
        }

        //Transparency
        if(transparency != phase.transparency)
        {
            float difference = phase.transparency - transparency;
            if(difference != 0)
            {
                float sign = 1;
                if(difference < 0)
                    sign = -1;

                float transmit = difference * sign * deltaTime_ / transmissionFactor;
                if(transmit < MIN_TRANSMISSION_VALUE)
                    transmit = MIN_TRANSMISSION_VALUE;

                transparency += transmit * sign;

                difference = phase.transparency - transparency;
                if(difference * sign < 0)
                    transparency = phase.transparency;
            }
        }
    }

    if(waitStartTime > 0)
    {
        waitStartTime -= deltaTime_;
        if(waitStartTime < 0)
            waitStartTime = 0;

        state = StartPhase;
        transparency = 0;
    }
    else if(state == StartPhase)
        state = DefaultPhase;
}

//DisplayText
GUI::DisplayText::DisplayText(std::string text_, sf::Font* font_, sf::Color color_, float size_, sf::Vector2f originFactor_,
                              DisplayPhase startPhase_, DisplayPhase defaultPhase_, DisplayPhase InactivePhase_,
                              float waitStartTime_, float waitEndTime_, float transmissionFactor_) :
    originFactor(originFactor_),
    stringRef(nullptr)
{
    if(font_ != nullptr)
        text.setFont(*font_);
    text.setCharacterSize(size_);
    text.setString(text_);
    text.setColor(color_);
    text.setOrigin(text.getLocalBounds().width * originFactor.x, text.getLocalBounds().height * originFactor.y);

    startPhase = startPhase_;
    defaultPhase = defaultPhase_;
    inactivePhase = InactivePhase_;
    waitStartTime = waitStartTime_;
    waitEndTime = waitEndTime_;
    startTime = waitStartTime_;
    endTime = waitEndTime_;
    transmissionFactor = transmissionFactor_;
}

GUI::DisplayText::DisplayText(std::wstring text_, sf::Font* font_, sf::Color color_, float size_, sf::Vector2f originFactor_,
                              DisplayPhase startPhase_, DisplayPhase defaultPhase_, DisplayPhase InactivePhase_,
                              float waitStartTime_, float waitEndTime_, float transmissionFactor_) :
    originFactor(originFactor_),
    stringRef(nullptr)
{
    if(font_ != nullptr)
        text.setFont(*font_);
    text.setCharacterSize(size_);
    text.setString(text_);
    text.setColor(color_);
    text.setOrigin(text.getLocalBounds().width * originFactor.x, text.getLocalBounds().height * originFactor.y);

    startPhase = startPhase_;
    defaultPhase = defaultPhase_;
    inactivePhase = InactivePhase_;
    waitStartTime = waitStartTime_;
    waitEndTime = waitEndTime_;
    startTime = waitStartTime_;
    endTime = waitEndTime_;
    transmissionFactor = transmissionFactor_;
}

GUI::DisplayText::DisplayText(std::string* stringRef_, sf::Font* font_, sf::Color color_, float size_, sf::Vector2f originFactor_,
                              DisplayPhase startPhase_, DisplayPhase defaultPhase_, DisplayPhase InactivePhase_,
                              float waitStartTime_, float waitEndTime_, float transmissionFactor_) :
    originFactor(originFactor_),
    stringRef(stringRef_)
{
    if(font_ != nullptr)
        text.setFont(*font_);
    text.setCharacterSize(size_);
    if(stringRef != nullptr)
        text.setString(*stringRef);
    text.setColor(color_);
    text.setOrigin(text.getLocalBounds().width * originFactor.x, text.getLocalBounds().height * originFactor.y);

    startPhase = startPhase_;
    defaultPhase = defaultPhase_;
    inactivePhase = InactivePhase_;
    waitStartTime = waitStartTime_;
    waitEndTime = waitEndTime_;
    startTime = waitStartTime_;
    endTime = waitEndTime_;
    transmissionFactor = transmissionFactor_;
}

GUI::DisplayText::DisplayText(std::string text_, sf::Font* font_, sf::Color color_, float size_,
                              DisplayPhase startPhase_, DisplayPhase defaultPhase_, DisplayPhase InactivePhase_,
                              float waitStartTime_, float waitEndTime_, float transmissionFactor_) :
    originFactor(0.5f, 0.5f),
    stringRef(nullptr)
{
    if(font_ != nullptr)
        text.setFont(*font_);
    text.setCharacterSize(size_);
    text.setString(text_);
    text.setColor(color_);
    text.setOrigin(text.getLocalBounds().width * originFactor.x, text.getLocalBounds().height * originFactor.y);

    startPhase = startPhase_;
    defaultPhase = defaultPhase_;
    inactivePhase = InactivePhase_;
    waitStartTime = waitStartTime_;
    waitEndTime = waitEndTime_;
    startTime = waitStartTime_;
    endTime = waitEndTime_;
    transmissionFactor = transmissionFactor_;
}

GUI::DisplayText::DisplayText(sf::Text text_,
                              DisplayPhase startPhase_, DisplayPhase defaultPhase_, DisplayPhase InactivePhase_,
                              float waitStartTime_, float waitEndTime_, float transmissionFactor_) :
    stringRef(nullptr)
{
    text = text_;
    originFactor = sf::Vector2f(text_.getOrigin().x / text_.getLocalBounds().width, text_.getOrigin().y / text_.getLocalBounds().height);

    startPhase = startPhase_;
    defaultPhase = defaultPhase_;
    inactivePhase = InactivePhase_;
    waitStartTime = waitStartTime_;
    waitEndTime = waitEndTime_;
    startTime = waitStartTime_;
    endTime = waitEndTime_;
    transmissionFactor = transmissionFactor_;
}

void GUI::DisplayText::Draw(sf::RenderTarget& target_)
{
    if(drawStatus && (int)transparency > 0)
    {
        if(stringRef != nullptr && *stringRef != text.getString())
        {
            text.setString(*stringRef);
            text.setOrigin(sf::Vector2f(text.getLocalBounds().width * originFactor.x, text.getLocalBounds().height * originFactor.y));
        }

        text.setPosition(position);
        text.setScale(scale);
        text.setColor(sf::Color(text.getColor().r, text.getColor().g, text.getColor().b, transparency));

        target_.draw(text);
    }
}

void GUI::DisplayText::SetText(std::string text_)
{
    stringRef = nullptr;
    text.setString(text_);
    text.setOrigin(sf::Vector2f(text.getLocalBounds().width * originFactor.x, text.getLocalBounds().height * originFactor.y));
}

void GUI::DisplayText::SetText(std::wstring text_)
{
    stringRef = nullptr;
    text.setString(text_);
    text.setOrigin(sf::Vector2f(text.getLocalBounds().width * originFactor.x, text.getLocalBounds().height * originFactor.y));
}

void GUI::DisplayText::SetText(std::string* stringRef_)
{
    if(stringRef_ != nullptr)
    {
        stringRef = stringRef_;
        text.setString(*stringRef);
        text.setOrigin(sf::Vector2f(text.getLocalBounds().width * originFactor.x, text.getLocalBounds().height * originFactor.y));
    }
}

//DisplayShape
GUI::DisplayShape::DisplayShape(sf::Vector2f rectangleSize_, sf::Color color_, sf::Vector2f originFactor_,
                  DisplayPhase startPhase_, DisplayPhase defaultPhase_, DisplayPhase InactivePhase_,
                  float waitStartTime_, float waitEndTime_, float transmissionFactor_)
{
    sf::RectangleShape rectangle;
    rectangle.setFillColor(color_);
    rectangle.setSize(rectangleSize_);
    rectangle.setOrigin(rectangleSize_.x * originFactor_.x, rectangleSize_.y * originFactor_.y);

    shape = new sf::RectangleShape;
    *shape = rectangle;
    color = color_;

    startPhase = startPhase_;
    defaultPhase = defaultPhase_;
    inactivePhase = InactivePhase_;
    waitStartTime = waitStartTime_;
    waitEndTime = waitEndTime_;
    startTime = waitStartTime_;
    endTime = waitEndTime_;
    transmissionFactor = transmissionFactor_;
}

GUI::DisplayShape::DisplayShape(sf::Vector2f rectangleSize_, sf::Color color_,
                  DisplayPhase startPhase_, DisplayPhase defaultPhase_, DisplayPhase InactivePhase_,
                  float waitStartTime_, float waitEndTime_, float transmissionFactor_)
{
    sf::RectangleShape rectangle;
    rectangle.setFillColor(color_);
    rectangle.setSize(rectangleSize_);
    rectangle.setOrigin(rectangleSize_ / 2.f);

    shape = new sf::RectangleShape;
    *shape = rectangle;
    color = color_;

    startPhase = startPhase_;
    defaultPhase = defaultPhase_;
    inactivePhase = InactivePhase_;
    waitStartTime = waitStartTime_;
    waitEndTime = waitEndTime_;
    startTime = waitStartTime_;
    endTime = waitEndTime_;
    transmissionFactor = transmissionFactor_;
}

GUI::DisplayShape::DisplayShape(float circleRadius, sf::Color color_,
             DisplayPhase startPhase_, DisplayPhase defaultPhase_, DisplayPhase InactivePhase_,
             float waitStartTime_, float waitEndTime_, float transmissionFactor_)
{
    sf::CircleShape circle(circleRadius);
    circle.setFillColor(color_);

    shape = new sf::CircleShape;
    *shape = circle;
    color = color_;

    startPhase = startPhase_;
    defaultPhase = defaultPhase_;
    inactivePhase = InactivePhase_;
    waitStartTime = waitStartTime_;
    waitEndTime = waitEndTime_;
    startTime = waitStartTime_;
    endTime = waitEndTime_;
    transmissionFactor = transmissionFactor_;
}

GUI::DisplayShape::DisplayShape(sf::ConvexShape shape_,
             DisplayPhase startPhase_, DisplayPhase defaultPhase_, DisplayPhase InactivePhase_,
             float waitStartTime_, float waitEndTime_, float transmissionFactor_)
{
    shape = new sf::ConvexShape;
    *shape = shape_;
    color = shape_.getFillColor();

    startPhase = startPhase_;
    defaultPhase = defaultPhase_;
    inactivePhase = InactivePhase_;
    waitStartTime = waitStartTime_;
    waitEndTime = waitEndTime_;
    startTime = waitStartTime_;
    endTime = waitEndTime_;
    transmissionFactor = transmissionFactor_;
}

GUI::DisplayShape::~DisplayShape()
{
    if(shape != nullptr)
        delete shape;
}

void GUI::DisplayShape::Draw(sf::RenderTarget& target_)
{
    if(drawStatus && (int)transparency > 0)
    {
        shape->setPosition(position);
        shape->setScale(scale);
        shape->setFillColor(sf::Color(color.r, color.g, color.b, color.a * transparency / 255.f));

        target_.draw(*shape);
    }
}

//DisplaySprite
GUI::DisplaySprite::DisplaySprite(sf::Texture* texture_,
                                  DisplayPhase startPhase_, DisplayPhase defaultPhase_, DisplayPhase InactivePhase_,
                                  float waitStartTime_, float waitEndTime_, float transmissionFactor_) :
    originFactor(0.5f, 0.5f)
{
    if(texture_ != nullptr)
    {
        sprite.setTexture(*texture_);
        sprite.setOrigin(sprite.getLocalBounds().width * originFactor.x, sprite.getLocalBounds().height * originFactor.y);
    }

    startPhase = startPhase_;
    defaultPhase = defaultPhase_;
    inactivePhase = InactivePhase_;
    waitStartTime = waitStartTime_;
    waitEndTime = waitEndTime_;
    startTime = waitStartTime_;
    endTime = waitEndTime_;
    transmissionFactor = transmissionFactor_;
}

GUI::DisplaySprite::DisplaySprite(sf::Texture* texture_, sf::IntRect textureRect_, sf::Vector2f originFactor_,
                                  DisplayPhase startPhase_, DisplayPhase defaultPhase_, DisplayPhase InactivePhase_,
                                  float waitStartTime_, float waitEndTime_, float transmissionFactor_) :
    DisplaySprite(texture_, textureRect_,
                  startPhase_, defaultPhase_, InactivePhase_,
                  waitStartTime_, waitEndTime_, transmissionFactor_)
{
    originFactor = originFactor_;
    sprite.setOrigin(sprite.getLocalBounds().width * originFactor.x, sprite.getLocalBounds().height * originFactor.y);
}

GUI::DisplaySprite::DisplaySprite(sf::Texture* texture_, sf::IntRect textureRect_,
                                  DisplayPhase startPhase_, DisplayPhase defaultPhase_, DisplayPhase InactivePhase_,
                                  float waitStartTime_, float waitEndTime_, float transmissionFactor_) :
    originFactor(0.5f, 0.5f)
{
    if(texture_ != nullptr)
    {
        sprite.setTexture(*texture_);
        sprite.setTextureRect(textureRect_);
        sprite.setOrigin(sprite.getLocalBounds().width * originFactor.x, sprite.getLocalBounds().height * originFactor.y);
    }

    startPhase = startPhase_;
    defaultPhase = defaultPhase_;
    inactivePhase = InactivePhase_;
    waitStartTime = waitStartTime_;
    waitEndTime = waitEndTime_;
    startTime = waitStartTime_;
    endTime = waitEndTime_;
    transmissionFactor = transmissionFactor_;
}

GUI::DisplaySprite::DisplaySprite(sf::Sprite& sprite_,
                                  DisplayPhase startPhase_, DisplayPhase defaultPhase_, DisplayPhase InactivePhase_,
                                  float waitStartTime_, float waitEndTime_, float transmissionFactor_)
{
    sprite = sprite_;
    originFactor = sf::Vector2f(sprite_.getOrigin().x / sprite_.getLocalBounds().width, sprite_.getOrigin().y / sprite_.getLocalBounds().height);

    startPhase = startPhase_;
    defaultPhase = defaultPhase_;
    inactivePhase = InactivePhase_;
    waitStartTime = waitStartTime_;
    waitEndTime = waitEndTime_;
    startTime = waitStartTime_;
    endTime = waitEndTime_;
    transmissionFactor = transmissionFactor_;
}

void GUI::DisplaySprite::Draw(sf::RenderTarget& target_)
{
    if(drawStatus && (int)transparency > 0)
    {
        sprite.setPosition(position);
        sprite.setScale(scale);
        sprite.setColor(sf::Color(sprite.getColor().r, sprite.getColor().g, sprite.getColor().b, transparency));

        target_.draw(sprite);
    }
}

void GUI::DisplaySprite::setTexture(sf::Texture* texture_, sf::IntRect rect_)
{
    if(texture_ != nullptr)
    {
        sprite.setTexture(*texture_);
        sprite.setTextureRect(rect_);
        sprite.setOrigin(sprite.getLocalBounds().width * originFactor.x, sprite.getLocalBounds().height * originFactor.y);
    }
}

void GUI::DisplaySprite::setTexture(sf::Texture* texture_)
{
    if(texture_ != nullptr)
    {
        sprite.setTexture(*texture_);
        sprite.setOrigin(sprite.getLocalBounds().width * originFactor.x, sprite.getLocalBounds().height * originFactor.y);
    }
}

void GUI::DisplaySprite::setTextureRect(sf::IntRect rect_)
{
    sprite.setTextureRect(rect_);
}

//DisplayVertexArray
GUI::DisplayVertexArray::DisplayVertexArray(sf::PrimitiveType primitiveType_, sf::Texture* texture_,
                                            DisplayPhase startPhase_, DisplayPhase defaultPhase_, DisplayPhase InactivePhase_,
                                            float waitStartTime_, float waitEndTime_, float transmissionFactor_) :
    texture(texture_)
{
    startPhase = startPhase_;
    defaultPhase = defaultPhase_;
    inactivePhase = InactivePhase_;
    waitStartTime = waitStartTime_;
    waitEndTime = waitEndTime_;
    startTime = waitStartTime_;
    endTime = waitEndTime_;
    transmissionFactor = transmissionFactor_;

    vertexArray.setPrimitiveType(primitiveType_);
    lastPosition = startPhase_.position;
    lastTransparency = -1;
}

void GUI::DisplayVertexArray::AddVertex(const sf::Vertex& vertex_)
{
    vertexArray.append(vertex_);
    vertexTransparency.push_back(vertex_.color.a);
}

void GUI::DisplayVertexArray::AddRectVertices(sf::FloatRect rect_, sf::Color color_, float rotation_, sf::IntRect textureRect_)
{
    sf::Vertex vertices[4];

    vertices[0].position  = sf::Vector2f(rect_.left, rect_.top);
    vertices[1].position  = sf::Vector2f(rect_.left + rect_.width, rect_.top);
    vertices[2].position  = sf::Vector2f(rect_.left + rect_.width, rect_.top + rect_.height);
    vertices[3].position  = sf::Vector2f(rect_.left, rect_.top + rect_.height);

    vertices[0].texCoords  = sf::Vector2f(textureRect_.left, textureRect_.top);
    vertices[1].texCoords  = sf::Vector2f(textureRect_.left + textureRect_.width, textureRect_.top);
    vertices[2].texCoords  = sf::Vector2f(textureRect_.left + textureRect_.width, textureRect_.top + textureRect_.height);
    vertices[3].texCoords  = sf::Vector2f(textureRect_.left, textureRect_.top + textureRect_.height);

    if(rotation_ != 0)
    {
        sf::Transformable transformable;
        transformable.setPosition(rect_.left + rect_.width / 2.f, rect_.top + rect_.height / 2.f);
        transformable.setOrigin(rect_.left + rect_.width / 2.f, rect_.top + rect_.height / 2.f);
        transformable.setRotation(rotation_);

        sf::Transform transform_ = transformable.getTransform();

        for(int a = 0; a < 4; a++)
            vertices[a].position = transform_.transformPoint(vertices[a].position);
    }

    for(int a = 0; a < 4; a++)
    {
        vertices[a].color = color_;
        vertexArray.append(vertices[a]);
        vertexTransparency.push_back(color_.a);
    }
}

void GUI::DisplayVertexArray::Draw(sf::RenderTarget& target_)
{
    if(drawStatus && (int)transparency > 0)
    {
        if(position != lastPosition)
        {
            sf::Vector2f diff = position - lastPosition;
            lastPosition = position;

            for(int a = 0; a < vertexTransparency.size(); a++)
                vertexArray[a].position += diff;
        }

        if(transparency != lastTransparency)
        {
            lastTransparency = transparency;

            for(int a = 0; a < vertexTransparency.size(); a++)
                vertexArray[a].color.a = vertexTransparency[a] * transparency / 255.f;
        }

        if(texture != nullptr)
            target_.draw(vertexArray, texture);
        else target_.draw(vertexArray);
    }
}

void GUI::DisplayVertexArray::EditVertext(const int& index_, const sf::Vector2f& position_, const sf::Color& color_, const sf::Vector2f& textrCoords_)
{
    if(index_ >= 0 && index_ < vertexArray.getVertexCount())
    {
        vertexArray[index_].position = position_;
        vertexArray[index_].color = color_;
        vertexArray[index_].color.a = (float)color_.a * transparency / 255.f;
        vertexArray[index_].texCoords = textrCoords_;

        vertexTransparency[index_] = color_.a;
    }
}

const sf::Vertex* GUI::DisplayVertexArray::GetVertex(const int& index_)
{
    if(index_ >= 0 && index_ < vertexArray.getVertexCount())
        return &vertexArray[index_];
    return nullptr;
}

void GUI::DisplayVertexArray::Resize(const int& size_)
{
    vertexArray.resize(size_);

    vertexTransparency.resize(size_);
    for(int a = 0; a < size_; a++)
        vertexTransparency[a] = vertexArray[a].color.a;
}

void GUI::DisplayVertexArray::Clear()
{
    vertexArray.clear();
    vertexTransparency.clear();
}
