import {DirectoryEntry} from '.';

/** Client used to access service locator services. */
export abstract class ServiceLocatorClient {

  /** Loads the directory entry representing the account currently logged
   *  in. */
  public abstract loadCurrentAccount(): Promise<DirectoryEntry>;

  /** Loads a directory entry from an id.
   * @param id - The id of the directory entry to load.
   * @return The directory entry with the specified id.
   */
  public abstract loadDirectoryEntryFromId(
    id: number): Promise<DirectoryEntry>;

  /** Logs into the service locator.
   * @param username - The username.
   * @param password - The password.
   * @return The DirectoryEntry of the account that logged in.
   * @throws ServiceError Indicates the log in failed.
   */
  public abstract login(username: string, password: string):
    Promise<DirectoryEntry>;

  /** Stores an account's password.
   * @param account The account to update.
   * @param password The account's updated password.
   */
  public abstract storePassword(account: DirectoryEntry, password: string):
    Promise<void>;

  /** Logs out of the service locator. */
  public abstract close(): Promise<void>;
}
