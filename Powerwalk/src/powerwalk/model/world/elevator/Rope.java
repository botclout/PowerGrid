/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package powerwalk.model.world.elevator;

import powerwalk.model.OutOfReachException;

/**
 *
 * @author Vincent W
 */
public class Rope extends Elevator{
    
    public Rope(int x,int y,int z,int rawValue,int[] otherPlanes) {
        super(x, y, z, rawValue, otherPlanes);
    }

    @Override
    public void move(int direction) throws OutOfReachException {
        throw new UnsupportedOperationException("Not supported yet.");
    }

    @Override
    public void interact() throws OutOfReachException {
        throw new UnsupportedOperationException("Not supported yet.");
    }

    @Override
    public void interact(String method) throws OutOfReachException {
        throw new UnsupportedOperationException("Not supported yet.");
    }
    
}
