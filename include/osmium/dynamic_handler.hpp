#ifndef OSMIUM_DYNAMIC_HANDLER_HPP
#define OSMIUM_DYNAMIC_HANDLER_HPP

/*

This file is part of Osmium (http://osmcode.org/libosmium).

Copyright 2013,2014 Jochen Topf <jochen@topf.org> and others (see README).

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

#include <memory>
#include <utility>

#include <osmium/handler.hpp>

namespace osmium {

    class Node;
    class Way;
    class Relation;
    class Area;
    class Changeset;

    namespace handler {

        namespace {

            class HandlerWrapperBase {

            public:

                virtual ~HandlerWrapperBase() {
                }

                virtual void node(const osmium::Node&) {
                }

                virtual void way(const osmium::Way&) {
                }

                virtual void relation(const osmium::Relation&) {
                }

                virtual void area(const osmium::Area&) {
                }

                virtual void changeset(const osmium::Changeset&) {
                }

                virtual void flush() {
                }

            }; // class HandlerWrapperBase


            // The following uses trick from
            // http://stackoverflow.com/questions/257288/is-it-possible-to-write-a-c-template-to-check-for-a-functions-existence
            // to either call handler style functions or visitor style operator().

#define HANDLER_DISPATCH1(_name_, _type_) \
template <class THandler> \
auto _name_##_dispatch(THandler& handler, const osmium::_type_& object, int) -> decltype(handler._name_(object), void()) { \
    handler._name_(object); \
} \
template <class THandler> \
auto _name_##_dispatch(THandler& handler, const osmium::_type_& object, long) -> decltype(handler(object), void()) { \
    handler(object); \
}

            HANDLER_DISPATCH1(node, Node);
            HANDLER_DISPATCH1(way, Way);
            HANDLER_DISPATCH1(relation, Relation);
            HANDLER_DISPATCH1(changeset, Changeset);
            HANDLER_DISPATCH1(area, Area);

            template <class THandler>
            auto flush_dispatch(THandler& handler, int) -> decltype(handler.flush(), void()) {
                handler.flush();
            }

            template <class THandler>
            void flush_dispatch(THandler&, long) {}

            template <class THandler>
            class HandlerWrapper : public HandlerWrapperBase {

                THandler m_handler;

            public:

                template <class... TArgs>
                HandlerWrapper(TArgs&&... args) :
                    m_handler(std::forward<TArgs>(args)...) {
                }

                void node(const osmium::Node& node) override final {
                    node_dispatch(m_handler, node, 0);
                }

                void way(const osmium::Way& way) override final {
                    way_dispatch(m_handler, way, 0);
                }

                void relation(const osmium::Relation& relation) override final {
                    relation_dispatch(m_handler, relation, 0);
                }

                void area(const osmium::Area& area) override final {
                    area_dispatch(m_handler, area, 0);
                }

                void changeset(const osmium::Changeset& changeset) override final {
                    changeset_dispatch(m_handler, changeset, 0);
                }

                void flush() override final {
                    flush_dispatch(m_handler, 0);
                }

            }; // HandlerWrapper

        } // anonymous namespace

        class DynamicHandler : public osmium::handler::Handler {

            typedef std::unique_ptr<HandlerWrapperBase> impl_ptr;
            impl_ptr m_impl;

        public:

            DynamicHandler() :
                m_impl(impl_ptr(new HandlerWrapperBase)) {
            }

            template <class THandler, class... TArgs>
            void set(TArgs&&... args) {
                m_impl = impl_ptr(new osmium::handler::HandlerWrapper<THandler>(std::forward<TArgs>(args)...));
            }

            void node(const osmium::Node& node) {
                m_impl->node(node);
            }

            void way(const osmium::Way& way) {
                m_impl->way(way);
            }

            void relation(const osmium::Relation& relation) {
                m_impl->relation(relation);
            }

            void area(const osmium::Area& area) {
                m_impl->area(area);
            }

            void changeset(const osmium::Changeset& changeset) {
                m_impl->changeset(changeset);
            }

            void flush() {
                m_impl->flush();
            }

        }; // DynamicHandler

    } // namspace handler

} // namespace osmium

#endif // OSMIUM_DYNAMIC_HANDLER_HPP
