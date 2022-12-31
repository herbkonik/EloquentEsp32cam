//
// Created by Simone on 31/12/22.
//

#ifndef ELOQUENTESP32CAM_DISPLAYSJPEGFEED_H
#define ELOQUENTESP32CAM_DISPLAYSJPEGFEED_H


#include <WebServer.h>
#include "../../Cam.h"
#include "./Resources.h"
#include "../../../traits/HasErrorMessage.h"


namespace Eloquent {
    namespace Esp32cam {
        namespace Http {
            namespace Features {

                /**
                 * Display live feed as a series of JPEGs
                 */
                class DisplaysJpegFeed : public HasErrorMessage {
                public:
                    WebServer *server;
                    Cam *cam;
                    Resources resources;

                    /**
                     *
                     * @param webServer
                     */
                    DisplaysJpegFeed(WebServer& webServer, Cam& camera) :
                        server(&webServer),
                        cam(&camera),
                        resources(webServer) {

                    }

                    /**
                     *
                     * @tparam Callback
                     * @param callback
                     */
                    template<typename Callback>
                    void onRequest(Callback callback) {
                        if (!probeCamera())
                            return;

                        server->on("/jpeg", [this, callback]() {
                            if (!cam->capture()) {
                                server->send(500, "text/plain", cam->getErrorMessage());
                                return;
                            }

                            WiFiClient client = server->client();

                            client.println(F("HTTP/1.1 200 OK"));
                            client.println(F("Content-Type: image/jpeg"));
                            client.println(F("Connection: close"));
                            client.print(F("Content-Length: "));
                            client.println(cam->frame->len);
                            client.println();
                            client.write(cam->frame->buf, cam->frame->len);

                            callback();
                        });
                    }

                    /**
                     *
                     */
                    void css() {
                        resources.css(F(R"===(
                            #frame {position: relative; display: inline-block;}
                            #canvas {position: relative; display: inline-block;}
                            canvas {transform-origin: 0 0;}
                        )==="));
                    }

                    /**
                     *
                     */
                    void js() {
                        resources.variable("imgWidth", cam->getWidth());
                        resources.variable("imgHeight", cam->getHeight());
                        resources.onDOMContentLoaded(F(R"===(
                            const jpeg = document.getElementById("jpeg")

                            function drawJpeg() {
                                try {
                                    fetch("/jpeg")
                                        .then(res => res.arrayBuffer())
                                        .then(buf => {
                                            const blob = new Blob([buf], { type: "image/jpeg" });

                                            jpeg.src = (window.URL || window.webkitURL).createObjectURL(blob)
                                            jpeg.style.width = `${APP.imgWidth}px`
                                            jpeg.style.height = `${APP.imgHeight}px`
                                            drawJpeg()
                                        })
                                } catch (e) { drawJpeg() }
                            }

                            drawJpeg()
                        )==="));
                    }

                protected:

                    /**
                     * Try to capture JPEG frame
                     *
                     * @return
                     */
                    bool probeCamera() {
                        if (!cam->capture())
                            return setErrorMessage("Cannot capture frame");

                        return setErrorMessage("");
                    }
                };
            }
        }
    }
}

#endif //ELOQUENTESP32CAM_DISPLAYSJPEGFEED_H
