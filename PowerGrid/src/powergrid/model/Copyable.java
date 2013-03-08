package powergrid.model;

/**
 * Model interface specifying objects that can be copied.
 * <p/>
 * It is parameterized so that it is possible to directly return the correct
 * object type without needing a cast.
 * <p/>
 * @author Chronio
 */
public interface Copyable<T> {
    /**
     * Copies this Copyable and returns the copy.
     * <p/>
     * Subclasses must make sure that for a given Copyable c, it holds that 
     * <code>c.equals(c.copy())</code> when the copy is possible. From this also 
     * follows that this method may not return null.
     * <p/>
     * It is allowed to throw an IllegalStateException when creating a copy is 
     * impossible or infeasible under the Copyable's circumstances at the time.
     * <p/>
     * @return a copy of this Copyable object
     * @throws IllegalStateException when creating a copy is not possible in the Object's state
     */
    public T copy();
}
