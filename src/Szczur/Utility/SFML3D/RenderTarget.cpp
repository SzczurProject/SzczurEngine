#include "RenderTarget.hpp"
#include "VertexArray.hpp"
#include "Drawable.hpp"
#include "Vertex.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>


namespace sf3d {

	RenderTarget::RenderTarget(const char* vertexPath, const char* fragmentPath, const glm::uvec2& size, float FOV) :
		_FOVy(FOV),
		_FOVx(
			glm::degrees(2 * glm::atan(glm::tan(glm::radians(FOV / 2.f)) * ((float)size.x / (float)size.y)))
		),
		_halfFOVxTan( glm::tan(glm::radians(_FOVx / 2.f)) ),
		_halfFOVyTan( glm::tan(glm::radians(_FOVy / 2.f)) ),
		_projection(1.f),
		_windowSize(size),
		_view(2.f / (float)size.y, {0.f, 0.f, 3 * (float)size.x / 2.f}),
		_defaultView(_view) {
		_projection = glm::perspective(glm::radians(FOV), (float)size.x / (float)size.y, 0.1f, 100.f);

		if(!gladLoadGL()) {
			std::cout << "Failed to initialize GLAD\n";
			while(true);
		}
		_shader = new Shader(vertexPath, fragmentPath);
		_states.shader = _shader;
	}

	RenderTarget::~RenderTarget() {
		delete _shader;
	}

	void RenderTarget::clear(float r, float g, float b, GLbitfield flags) {
		;
		if(_setActive()) {
			glClearColor(r / 255.f, g / 255.f, b / 255.f, 1.f);
			glClear(flags);
			glBindFramebuffer(GL_FRAMEBUFFER, NULL);
		}
	}

	void RenderTarget::draw(const Drawable & drawable, RenderStates states) {
		drawable.draw(*this, states);
	}

	void RenderTarget::draw(const Drawable & drawable) {
		draw(drawable, _states);
	}



	void RenderTarget::draw(const VertexArray& vertices, RenderStates states) {
		if(vertices.getSize() > 0 && _setActive()) {
			if(states.shader)
				states.shader->use();
			else
				_states.shader->use();

			for(int i = 0; i < 3; i++)
				states.transform.getMatrix()[3][i] *= 2.f / (float)_windowSize.y;

			glUniform1f(
				glGetUniformLocation(states.shader->ID, "positionFactor"),
				2.f / (float)_windowSize.y
			);

			glUniformMatrix4fv(
				glGetUniformLocation(states.shader->ID, "model"),
				1, GL_FALSE, glm::value_ptr(states.transform.getMatrix())
			);

			glUniformMatrix4fv(
				glGetUniformLocation(states.shader->ID, "view"),
				1, GL_FALSE, glm::value_ptr(_view.getTransform().getMatrix())
			);

			glUniformMatrix4fv(
				glGetUniformLocation(states.shader->ID, "projection"),
				1, GL_FALSE, glm::value_ptr(_projection)
			);

			glUniform1i(
				glGetUniformLocation(states.shader->ID, "isTextured"),
				(states.texture) ? 1 : 0
			);

			if(states.texture)
				states.texture->bind();

			vertices.bind();

			glDrawArrays(vertices.getPrimitiveType(), 0, vertices.getSize());
			states.texture->unbind();
			glBindVertexArray(NULL);

			glBindFramebuffer(GL_FRAMEBUFFER, NULL);
		}
	}

	void RenderTarget::simpleDraw(const VertexArray & vertices, RenderStates states) {
		if(vertices.getSize() > 0 && _setActive()) {
			if(states.shader)
				states.shader->use();


			if(states.texture)
				states.texture->bind();

			vertices.bind();

			glDrawArrays(vertices.getPrimitiveType(), 0, vertices.getSize());

			states.texture->unbind();
			glBindVertexArray(NULL);
			glBindFramebuffer(GL_FRAMEBUFFER, NULL);
		}
	}

	void RenderTarget::draw(const VertexArray & vertices) {
		draw(vertices, _states);
	}

	void RenderTarget::simpleDraw(const VertexArray & vertices) {
		simpleDraw(vertices, _states);
	}

	void RenderTarget::simpleDraw(Drawable & drawable) {
		draw(drawable, _states);
	}

	const View& RenderTarget::getDefaultView() const {
		return _defaultView;
	}

	const View& RenderTarget::getView() const {
		return _view;
	}

	void RenderTarget::setView(const View& view) {
		_view = view;
	}

	Linear RenderTarget::getLinerByScreenPos(const glm::vec2 & pos) const {
		float x = glm::atan(
			( 2.f * (pos.x / (float)_windowSize.x) - 1.f) * _halfFOVxTan
		);

		float y = glm::atan(
			(-2.f * (pos.y / (float)_windowSize.y) + 1.f) * _halfFOVyTan
		);

		float siny = glm::sin(y);
		float cosy = glm::cos(y);
		float sinx = glm::sin(x);
		float cosx = glm::cos(x);

		glm::vec3 rotation{
			cosy * sinx, 
			siny * cosx, 
			-cosy * cosx
		};
		rotation = glm::rotateX(rotation, glm::radians(-_view.getRotation().x));
		rotation = glm::rotateY(rotation, glm::radians(-_view.getRotation().y));
		return Linear(_view.getCenter(), {
			rotation.x,
			rotation.y,
			rotation.z
		});
	}

	bool RenderTarget::_setActive(bool state) {
		glBindFramebuffer(GL_FRAMEBUFFER, NULL);
		return true;
	}


}