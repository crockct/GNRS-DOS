package edu.rutgers.winlab.mfirst.net.ipv4udp;

import edu.rutgers.winlab.mfirst.messages.UpdateResponseMessage;
import edu.rutgers.winlab.mfirst.messages.opt.Option;
import org.apache.mina.core.buffer.IoBuffer;
import org.apache.mina.core.session.IoSession;
import org.apache.mina.filter.codec.ProtocolEncoderOutput;
import org.apache.mina.filter.codec.demux.MessageEncoder;

import java.util.List;

/**
 * Created by wontoniii on 4/24/15.
 */
public class UpdateResponseEncoder implements MessageEncoder<UpdateResponseMessage> {


    @Override
    public void encode(final IoSession session, final UpdateResponseMessage message,
                       final ProtocolEncoderOutput out) {
        // Common Response stuff
        final IoBuffer buffer = IoBuffer.allocate(message.getMessageLength());
        buffer.put(message.getVersion());
        buffer.put(message.getType().value());
        buffer.putUnsignedShort(message.getMessageLength());
        buffer.putUnsignedInt(message.getRequestId());

        // Offset values
        int optionsOffset = 0;
        // 12 + address T&L + address length
        int payloadOffset = 16 + message.getOriginAddress().getLength();
        if (!message.getOptions().isEmpty()) {
            optionsOffset = payloadOffset + message.getPayloadLength();
        }
        buffer.putUnsignedShort(optionsOffset);
        buffer.putUnsignedShort(payloadOffset);

        // Address
        buffer.putUnsignedShort(message.getOriginAddress().getType().value());
        buffer.putUnsignedShort(message.getOriginAddress().getLength());
        buffer.put(message.getOriginAddress().getValue());

        buffer.putUnsignedShort(message.getResponseCode().value());
        // Padding
        buffer.putUnsignedShort(0);

        List<Option> options = message.getOptions();
        if(options != null && !options.isEmpty()){
            buffer.put(RequestOptionsTranscoder.encode(options));
        }

        buffer.flip();
        out.write(buffer);
    }

}