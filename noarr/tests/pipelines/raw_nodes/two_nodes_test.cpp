#include <catch2/catch.hpp>

#include <cstddef>
#include <iostream>

#include <noarr/pipelines/Device.hpp>
#include <noarr/pipelines/Envelope.hpp>
#include <noarr/pipelines/DebuggingScheduler.hpp>

#include "MyProducingNode.hpp"
#include "MyConsumingNode.hpp"

using namespace noarr::pipelines;

TEST_CASE("Two nodes", "[node]") {

    // create an envelope
    char buffer[1024];
    auto s = Envelope<std::size_t, char>(Device(-1), buffer, 1024);

    // create our nodes
    auto prod = MyProducingNode("lorem ipsum", 3);
    auto cons = MyConsumingNode();

    // link those nodes together
    prod.output_port.send_processed_envelopes_to(&cons.input_port);
    cons.input_port.send_processed_envelopes_to(&prod.output_port);

    // put the envelope into the producer
    prod.output_port.attach_envelope(&s);

    // setup a scheduler
    auto scheduler = DebuggingScheduler(std::cout);
    scheduler.add(prod);
    scheduler.add(cons);
    
    SECTION("they can cycle an envelope") {
        // run the pipeline to completion
        scheduler.run();
        
        // assert the string has been transfered
        REQUIRE(cons.received_string == "lorem ipsum");
    }
}
