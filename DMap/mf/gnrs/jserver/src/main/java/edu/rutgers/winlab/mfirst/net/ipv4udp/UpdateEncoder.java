package edu.rutgers.winlab.mfirst.net.ipv4udp;

import edu.rutgers.winlab.mfirst.messages.UpdateMessage;
import edu.rutgers.winlab.mfirst.messages.opt.Option;
import edu.rutgers.winlab.mfirst.net.NetworkAddress;
import org.apache.mina.core.buffer.IoBuffer;
import org.apache.mina.core.session.IoSession;
import org.apache.mina.filter.codec.ProtocolEncoderOutput;
import org.apache.mina.filter.codec.demux.MessageEncoder;

import java.util.List;

/**
 * Created by wontoniii on 4/24/15.
 */
public class UpdateEncoder implements MessageEncoder<UpdateMessage> {

    @Override
    public void encode(final IoSession session, final UpdateMessage message,
                       final ProtocolEncoderOutput out) {

        final IoBuffer buff = IoBuffer.allocate(message.getMessageLength());

        // Generic request stuff
        buff.put(message.getVersion());
        buff.put(message.getType().value());
        buff.putUnsignedShort(message.getMessageLength());

        buff.putUnsignedInt(message.getRequestId());

        // Offset values
        int optionsOffset = 0;
        // 12 + address T&L + address length
        int payloadOffset = 16 + message.getOriginAddress().getLength();
        if (!message.getOptions().isEmpty()) {
            optionsOffset = payloadOffset + message.getPayloadLength();
        }
        buff.putUnsignedShort(optionsOffset);
        buff.putUnsignedShort(payloadOffset);

        buff.putUnsignedShort(message.getOriginAddress().getType().value());
        buff.putUnsignedShort(message.getOriginAddress().getLength());
        buff.put(message.getOriginAddress().getValue());

        // Specific for update messages
        buff.put(message.getGuid().getBinaryForm());

        buff.putUnsignedInt(message.getNumBindings());
        if (message.getNumBindings() > 0) {
            for (final NetworkAddress addx : message.getBindings()) {
                buff.putUnsignedShort(addx.getType().value());
                buff.putUnsignedShort(addx.getLength());
                buff.put(addx.getValue());
            }
        }

        List<Option> options = message.getOptions();
        if (options != null && !options.isEmpty()) {
            buff.put(RequestOptionsTranscoder.encode(options));
        }

        buff.flip();
        out.write(buff);

    }

}